/*
 * expr_real_const.cpp
 *
 *  Created on: Oct 7, 2016
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
    
    static const int classID = LOGIC_EXPR_REAL_CONST;
    LogicExpressionRealConstant::LogicExpressionRealConstant(string value, NodeSource* source) : LogicExpression(source), value{value} {
        id = classID;
        
        if (find(this->value.begin(), this->value.end(), '.') == this->value.end()) {
            this->value += ".0";
        }
        
        if (this->value[0] == '-') {
            this->value = "(-. " + this->value.substr(1) + ")";
        }
    }
    LogicExpressionRealConstant::~LogicExpressionRealConstant() {}
    
    string LogicExpressionRealConstant::getValue() {
        return value;
    }
    
    string LogicExpressionRealConstant::toString() {
        return value;
    }
    
    static LogicTypeReal retType;
    LogicType* LogicExpressionRealConstant::returnType() {
        return &retType;
    }
    
    void LogicExpressionRealConstant::checkTypes() {}
    
    void LogicExpressionRealConstant::toWhy3(ostream &out, Why3Data &data) {
        out << value;
    }
    
    bool LogicExpressionRealConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
