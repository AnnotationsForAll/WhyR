/*
 * instruction.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    AnnotatedInstruction::AnnotatedInstruction(AnnotatedFunction* function, Instruction* llvm) : function{function}, llvm{llvm} {}
    AnnotatedInstruction::~AnnotatedInstruction() {
        if (assert) delete assert;
        if (assume) delete assume;
    }
    
    void AnnotatedInstruction::annotate() {
        unsigned assumeKind = llvm->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.assume")));
        unsigned assertKind = llvm->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.assert")));
        unsigned labelKind = llvm->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.label")));
        
        // find possible WhyR metadata on this instruction
        SmallVector<pair<unsigned, MDNode*>, 1> sv;
        llvm->getAllMetadata(sv);
        for (SmallVector<pair<unsigned, MDNode*>, 1>::iterator ii = sv.begin(); ii != sv.end(); ii++) {
            unsigned kind = (*ii).first;
            MDNode* node = (*ii).second;
            
            try {
                // handle assumes and asserts nodes the same, but assign them to different locations
                // These nodes are predicates, having the bool return type.
                if (kind == assumeKind || kind == assertKind) {
                    if (node->getNumOperands() != 1) {
                        throw syntax_exception("got " + to_string(node->getNumOperands()) + " operands to " + (kind == assumeKind ? "assume" : "assert" ) + " clause; expected 1", NULL, new NodeSource(getFunction(), this->rawIR(), NULL));
                    }
                    
                    LogicExpression* expr = ExpressionParser::parseMetadata(node->getOperand(0), new NodeSource(getFunction(), this->rawIR(), node->getOperand(0).get()));
                    expr->checkTypes();
                    
                    if (!isa<LogicTypeBool>(expr->returnType())) {
                        throw type_exception(string(kind == assumeKind ? "assume" : "assert" ) + " clause requires an expression of type 'bool'; got type '" + expr->returnType()->toString() + "'", NULL, new NodeSource(getFunction(), this->rawIR(), node->getOperand(0).get()));
                    }
                    
                    if (kind == assumeKind) {
                        assume = expr;
                    } else {
                        assert = expr;
                    }
                }
                // Labels are metadata strings.
                if (kind == labelKind) {
                    if (node->getNumOperands() != 1) {
                        throw syntax_exception("got " + to_string(node->getNumOperands()) + " operands to label; expected 1", NULL, new NodeSource(getFunction(), this->rawIR(), NULL));
                    }
                    
                    if (isa<MDString>(node->getOperand(0).get())) {
                        const char* labelName = cast<MDString>(node->getOperand(0).get())->getString().data();
                        label = labelName;
                    } else {
                        throw syntax_exception("Label name must be a metadata string", NULL, new NodeSource(getFunction(), this->rawIR(), node->getOperand(0).get()));
                    }
                }
            } catch (whyr_exception &ex) {
                if (!getFunction()->getModule()->getSettings()) throw ex;
                getFunction()->getModule()->getSettings()->errors.push_back(ex);
            }
        }
    }
    
    Instruction* AnnotatedInstruction::rawIR() {
        return llvm;
    }
    
    AnnotatedFunction* AnnotatedInstruction::getFunction() {
        return function;
    }
    
    LogicExpression* AnnotatedInstruction::getAssumeClause() {
        return assume;
    }
    
    LogicExpression* AnnotatedInstruction::getAssertClause() {
        return assert;
    }
    
    const char* AnnotatedInstruction::getLabel() {
        return label;
    }
    
    bool AnnotatedInstruction::isAnnotated(Instruction* inst) {
        unsigned assumeKind = inst->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.assume")));
        unsigned assertKind = inst->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.assert")));
        unsigned labelKind = inst->getParent()->getParent()->getParent()->getMDKindID(StringRef(string("whyr.label")));
        
        // if we have any kind of WhyR annotation node, yes, we need to create an AnnotatedInstruction for this
        SmallVector<pair<unsigned, MDNode*>, 1> sv;
        inst->getAllMetadata(sv);
        for (SmallVector<pair<unsigned, MDNode*>, 1>::iterator ii = sv.begin(); ii != sv.end(); ii++) {
            unsigned kind = (*ii).first;
            
            if (kind == assumeKind || kind == assertKind || kind == labelKind) {
                return true;
            }
        }
        // if no annotations exist, there is no need to store an AnnotatedInstruction
        return false;
    }
    
    LogicExpression* AnnotatedInstruction::setAssumeClause(LogicExpression* expr) {
        LogicExpression* old = assume;
        assume = expr;
        return old;
    }
    
    LogicExpression* AnnotatedInstruction::setAssertClause(LogicExpression* expr) {
        LogicExpression* old = assert;
        assert = expr;
        return old;
    }
}
