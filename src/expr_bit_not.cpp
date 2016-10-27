/*
 * expr_bit_not.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_BIT_NOT;
    LogicExpressionBitNot::LogicExpressionBitNot(LogicExpression* rhs, NodeSource* source) : LogicExpression(source), rhs{rhs} {
        id = classID;
    }
    LogicExpressionBitNot::~LogicExpressionBitNot() {
        delete rhs;
    }
    
    LogicExpression* LogicExpressionBitNot::getValue() {
        return rhs;
    }
    
    string LogicExpressionBitNot::toString() {
        return "~" + rhs->toString();
    }
    
    LogicType* LogicExpressionBitNot::returnType() {
        return rhs->returnType();
    }
    
    void LogicExpressionBitNot::checkTypes() {
        rhs->checkTypes();
        
        // The operand needs to be an LLVM int
        if (!isa<LogicTypeLLVM>(returnType()) || !cast<LogicTypeLLVM>(returnType())->getType()->isIntOrIntVectorTy()) {
            throw type_exception(("Operator 'bnot' undefined for non-numeric type '" + returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBitNot::toWhy3(ostream &out, Why3Data &data) {
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".bw_not ";
        rhs->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionBitNot::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
