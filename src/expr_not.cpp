/*
 * expr_bool_const.cpp
 *
 *  Created on: Sep 16, 2016
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
    
    static const int classID = LOGIC_EXPR_NOT;
    LogicExpressionNot::LogicExpressionNot(LogicExpression* rhs, NodeSource* source) : LogicExpression(source), rhs{rhs} {
        id = classID;
    }
    LogicExpressionNot::~LogicExpressionNot() {
        delete rhs;
    }
    
    LogicExpression* LogicExpressionNot::getValue() {
        return rhs;
    }
    
    string LogicExpressionNot::toString() {
        return "!" + rhs->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionNot::returnType() {
        return &retType;
    }
    
    void LogicExpressionNot::checkTypes() {
        rhs->checkTypes();
        
        // both arguments must be boolean
        if (!LogicType::commonType(rhs->returnType(), &retType)) {
            throw type_exception(("Operator 'not' undefined on type '" + rhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionNot::toWhy3(ostream &out, Why3Data &data) {
        out << "(not ";
        rhs->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionNot::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
