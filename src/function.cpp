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
                        
                        LogicTypeBool boolType;
                        if (!LogicType::commonType(expr->returnType(), &boolType)) {
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
                        for (unsigned i = 0; i < node->getNumOperands(); i++) {
                            Metadata* subnode = node->getOperand(i).get();
                            LogicExpression* expr = ExpressionParser::parseMetadata(subnode, new NodeSource(this, NULL, node->getOperand(0).get()));
                            expr->checkTypes();
                            
                            // a void* pointer with a address space of -1 represents a pointer to anything in WhyR.
                            // Note that void* pointers are illegal in LLVM proper.
                            LogicTypeLLVM ptrType(PointerType::get(Type::getVoidTy(llvm->getContext()), -1));
                            LogicTypeSet setType(&ptrType);
                            if (LogicType::commonType(expr->returnType(), &setType)) {
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
        if (!assigns.empty()) {
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
