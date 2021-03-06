/*
 * function.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/exception.hpp>
#include <whyr/types.hpp>
#include <whyr/expressions.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    AnnotatedFunction::AnnotatedFunction(AnnotatedModule* module, Function* llvm) : module{module}, llvm{llvm} {}
    
    AnnotatedFunction::~AnnotatedFunction() {
        delete requires;
        delete ensures;
        
        for (list<LogicExpression*>::iterator ii = assigns.begin(); ii != assigns.end(); ii++) {
            delete *ii;
        }
        
        for (list<AnnotatedInstruction*>::iterator ii = annotatedInsts.begin(); ii != annotatedInsts.end(); ii++) {
            delete *ii;
        }
    }
    
    static void addAssignsAssertions(AnnotatedFunction* func) {
        for (Function::iterator ii = func->rawIR()->begin(); ii != func->rawIR()->end(); ii++) {
            for (BasicBlock::iterator jj = ii->begin(); jj != ii->end(); jj++) {
                // if we are a store instruction, we need to be annotated with our assigns clause,
                // so we assert that we do not modify any memory not in the set
                if (isa<StoreInst>(&*jj)) {
                    NodeSource* src = new NodeSource(func, &*jj);
                    src->label = "assigns";
                    
                    LogicExpression* expr = NULL;
                    for (list<LogicExpression*>::iterator kk = func->getAssignsLocations()->begin(); kk != func->getAssignsLocations()->end(); kk++) {
                        LogicExpression* inExpr = new LogicExpressionInSet(
                                *kk,
                                new LogicExpressionLLVMOperand(jj->getOperand(1),src)
                        ,src);
                        
                        if (expr) {
                            inExpr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_OR,
                                    expr,
                                    inExpr
                            ,src);
                        }
                        expr = inExpr;
                    }
                    
                    // it is also acceptable to assign a location if it is allocated after the function's entry point.
                    expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_OR,
                            new LogicExpressionOld(
                                    new LogicExpressionFresh(false,
                                            new LogicExpressionLLVMOperand(jj->getOperand(1),src)
                                    ,src)
                            ,src),
                            expr
                    ,src);
                    
                    AnnotatedInstruction* inst = func->getAnnotatedInstruction(&*jj);
                    if (inst) {
                        if (inst->getAssertClause()) {
                            inst->setAssertClause(new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                                    expr,
                                    inst->getAssertClause()
                            ,src));
                        } else {
                            inst->setAssertClause(expr);
                        }
                    } else {
                        inst = new AnnotatedInstruction(func, &*jj);
                        func->getAnnotatedInstructions()->push_back(inst);
                        inst->setAssertClause(expr);
                    }
                } if (isa<CallInst>(&*jj)) {
                    Function* calledFuncRaw = cast<CallInst>(&*jj)->getCalledFunction();
                    if (!calledFuncRaw) continue;
                    AnnotatedFunction* calledFunc = func->getModule()->getFunction(calledFuncRaw);
                    if (!calledFunc) continue;
                    
                    NodeSource* src = new NodeSource(func, &*jj);
                    src->label = "assigns";
                    LogicExpression* expr = NULL;
                    
                    if (calledFunc->getAssignsLocations()) {
                        // add the assertion that the called function doesn't assign to anything we can't.
                        
                        // forall x : a_memb. (mem x a) -> (mem x b || mem x c || ...)
                        for (list<LogicExpression*>::iterator kk = func->getAssignsLocations()->begin(); kk != func->getAssignsLocations()->end(); kk++) {
                            NodeSource* newSource = new NodeSource(src);
                            LogicLocal* local = new LogicLocal(); local->name = "elem"; local->type = cast<LogicTypeSet>((*kk)->returnType())->getType();
                            newSource->logicLocals[local->name].push_front(local);
                            
                            LogicExpression* orExpr = NULL;
                            for (list<LogicExpression*>::iterator ll = calledFunc->getAssignsLocations()->begin(); ll != calledFunc->getAssignsLocations()->end(); ll++) {
                                LogicExpression* inExpr = new LogicExpressionInSet(
                                        *ll,
                                        new LogicExpressionLocal("elem", newSource)
                                ,newSource);
                                
                                if (orExpr) {
                                    orExpr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_OR,
                                            orExpr,
                                            inExpr
                                    ,newSource);
                                } else {
                                    orExpr = inExpr;
                                }
                            }
                            
                            LogicExpression* forallExpr = new LogicExpressionQuantifier(true,
                                    new list<LogicLocal*>({local}),
                                    new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_IMPLIES,
                                            new LogicExpressionInSet(
                                                    *kk,
                                                    new LogicExpressionLocal("elem", newSource)
                                            ,newSource),
                                            orExpr
                                    ,newSource)
                            ,newSource);
                            
                            if (expr) {
                                expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                                        expr,
                                        forallExpr
                                ,newSource);
                            } else {
                                expr = forallExpr;
                            }
                        }
                    } else {
                        // if the called function assigns everything, it can always assign something we can't.
                        // this equates to being unprovable- that is, false.
                        expr = new LogicExpressionBooleanConstant(false, src);
                    }
                    
                    AnnotatedInstruction* inst = func->getAnnotatedInstruction(&*jj);
                    if (inst) {
                        if (inst->getAssertClause()) {
                            inst->setAssertClause(new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                                    expr,
                                    inst->getAssertClause()
                            ,src));
                        } else {
                            inst->setAssertClause(expr);
                        }
                    } else {
                        inst = new AnnotatedInstruction(func, &*jj);
                        func->getAnnotatedInstructions()->push_back(inst);
                        inst->setAssertClause(expr);
                    }
                }
            }
        }
    }
    
    void AnnotatedFunction::annotate() {
        // check if we have WhyR metadata
        if (llvm->hasMetadata()) {
            unsigned requiresKind = llvm->getParent()->getMDKindID(StringRef(string("whyr.requires")));
            unsigned ensuresKind = llvm->getParent()->getMDKindID(StringRef(string("whyr.ensures")));
            unsigned assignsKind = llvm->getParent()->getMDKindID(StringRef(string("whyr.assigns")));
            
            SmallVector<pair<unsigned, MDNode*>, 1> sv;
            llvm->getAllMetadata(sv);
            for (SmallVector<pair<unsigned, MDNode*>, 1>::iterator ii = sv.begin(); ii != sv.end(); ii++) {
                unsigned kind = (*ii).first;
                MDNode* node = (*ii).second;
                
                try {
                    // handle requires and ensures nodes the same, but assign them to different locations
                    // These nodes are predicates, having the bool return type.
                    if (kind == requiresKind || kind == ensuresKind) {
                        if (node->getNumOperands() != 1) {
                            throw syntax_exception("got " + to_string(node->getNumOperands()) + " operands to " + (kind == requiresKind ? "requires" : "ensures" ) + " clause; expected 1", NULL, new NodeSource(this));
                        }
                        LogicExpression* expr = ExpressionParser::parseMetadata(node->getOperand(0), new NodeSource(this, NULL, node->getOperand(0).get()));
                        expr->checkTypes();
                        
                        if (!isa<LogicTypeBool>(expr->returnType())) {
                            throw type_exception((string(kind == requiresKind ? "requires" : "ensures" ) + " clause requires an expression of type 'bool'; got type '" + expr->returnType()->toString() + "'"), NULL, new NodeSource(this));
                        }
                        
                        if (kind == requiresKind) {
                            requires = expr;
                        } else {
                            ensures = expr;
                        }
                    }
                    
                    // An assigns node is a list of sets to any pointer type.
                    if (kind == assignsKind) {
                        hasAssgins = true;
                        
                        for (unsigned i = 0; i < node->getNumOperands(); i++) {
                            Metadata* subnode = node->getOperand(i).get();
                            LogicExpression* expr = ExpressionParser::parseMetadata(subnode, new NodeSource(this, NULL, node->getOperand(0).get()));
                            expr->checkTypes();
                            
                            if (isa<LogicTypeSet>(expr->returnType()) && isa<LogicTypeLLVM>(cast<LogicTypeSet>(expr->returnType())->getType()) && cast<LogicTypeLLVM>(cast<LogicTypeSet>(expr->returnType())->getType())->getType()->isPointerTy()) {
                                assigns.push_back(expr);
                            } else {
                                throw type_exception(("assigns clause requires expressions of type 'set<void*>'; got an expression of type '" + expr->returnType()->toString() + "'"), NULL, new NodeSource(this));
                            }
                        }
                    }
                } catch (whyr_exception &ex) {
                    if (!getModule()->getSettings()) throw ex;
                    getModule()->getSettings()->errors.push_back(ex);
                }
            }
        }
        
        // Check if any instructions have WhyR metadata
        for (Function::iterator ii = llvm->begin(); ii != llvm->end(); ii++) {
            BasicBlock* block = &*ii;
            for (BasicBlock::iterator jj = block->begin(); jj != block->end(); jj++) {
                Instruction* inst = &*jj;
                if (AnnotatedInstruction::isAnnotated(inst)) {
                    AnnotatedInstruction* annInst = new AnnotatedInstruction(this, inst);
                    annInst->annotate();
                    annotatedInsts.push_back(annInst);
                }
            }
        }
        
        // add assigns assertions if we need to
        if (hasAssgins) {
            addAssignsAssertions(this);
        }
    }
    
    Function* AnnotatedFunction::rawIR() {
        return llvm;
    }
    
    LogicExpression* AnnotatedFunction::getRequiresClause() {
        return requires;
    }
    
    LogicExpression* AnnotatedFunction::getEnsuresClause() {
        return ensures;
    }
    
    list<LogicExpression*>* AnnotatedFunction::getAssignsLocations() {
        if (!hasAssgins) {
            return NULL;
        }
        return &assigns;
    }
    
    AnnotatedModule* AnnotatedFunction::getModule() {
        return module;
    }
    
    list<AnnotatedInstruction*>* AnnotatedFunction::getAnnotatedInstructions() {
        return &annotatedInsts;
    }
    
    AnnotatedInstruction* AnnotatedFunction::getAnnotatedInstruction(Instruction* inst) {
        for (list<AnnotatedInstruction*>::iterator ii = annotatedInsts.begin(); ii != annotatedInsts.end(); ii++) {
            if ((*ii)->rawIR() == inst) {
                return *ii;
            }
        }
        return NULL;
    }
}
