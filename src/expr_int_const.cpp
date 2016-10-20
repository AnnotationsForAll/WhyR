/*
 * expr_int_const.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_INT_CONST;
    LogicExpressionIntegerConstant::LogicExpressionIntegerConstant(string value, NodeSource* source) : LogicExpression(source), value{value} {
        id = classID;
        
        if (this->value[0] == '-') {
            this->value = "(" + this->value + ")";
        }
    }
    LogicExpressionIntegerConstant::~LogicExpressionIntegerConstant() {}
    
    string LogicExpressionIntegerConstant::getValue() {
        return value;
    }
    
    string LogicExpressionIntegerConstant::toString() {
        return value;
    }
    
    static LogicTypeInt retType;
    LogicType* LogicExpressionIntegerConstant::returnType() {
        return &retType;
    }
    
    void LogicExpressionIntegerConstant::checkTypes() {}
    
    void LogicExpressionIntegerConstant::toWhy3(ostream &out, Why3Data &data) {
        out << value;
    }
    
    bool LogicExpressionIntegerConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
