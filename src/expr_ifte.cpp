/*
 * expr_ifte.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_IFTE;
    LogicExpressionConditional::LogicExpressionConditional(LogicExpression* condition, LogicExpression* ifTrue, LogicExpression* ifFalse, NodeSource* source) : LogicExpression(source), condition{condition}, ifTrue{ifTrue}, ifFalse{ifFalse} {
        id = classID;
        retType = LogicType::commonType(ifTrue->returnType(), ifFalse->returnType());
    }
    
    LogicExpressionConditional::~LogicExpressionConditional() {
        delete condition;
        delete ifTrue;
        delete ifFalse;
    }
    
    LogicExpression* LogicExpressionConditional::getCondition() {
        return condition;
    }
    
    LogicExpression* LogicExpressionConditional::getIfTrue() {
        return ifTrue;
    }
    
    LogicExpression* LogicExpressionConditional::getIfFalse() {
        return ifFalse;
    }
    
    LogicType* LogicExpressionConditional::returnType() {
        return retType;
    }
    
    string LogicExpressionConditional::toString() {
        return condition->toString() + " ? " + ifTrue->toString() + " : " + ifFalse->toString();
    }
    
    void LogicExpressionConditional::checkTypes() {
        condition->checkTypes();
        ifTrue->checkTypes();
        ifFalse->checkTypes();
        
        // condition must be boolean
        if (!isa<LogicTypeBool>(condition->returnType())) {
            throw type_exception(("Condition of operator 'ifte' must be of type 'bool'; got type '" + condition->returnType()->toString() + "'"), this);
        }
        
        // ifTrue and ifFalse must be of the same type
        if (!ifTrue->returnType()->equals(ifFalse->returnType())) {
            throw type_exception(("Operator 'ifte' undefined between branch types '" + ifTrue->returnType()->toString() + "' and '" + ifFalse->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionConditional::toWhy3(ostream &out, Why3Data &data) {
        out << "(if ";
        condition->toWhy3(out, data);
        out << " then ";
        ifTrue->toWhy3(out, data);
        out << " else ";
        ifFalse->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionConditional::classof(const LogicExpression* type) {
        return type->id == classID;
    }
}
