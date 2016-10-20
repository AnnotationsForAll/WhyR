/*
 * expr_result.cpp
 *
 *  Created on: Sep 16, 2016
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
    
    static const int classID = LOGIC_EXPR_RESULT;
    LogicExpressionResult::LogicExpressionResult(LogicType* type, NodeSource* source) : LogicExpression(source), type{type} {
        id = classID;
    }
    LogicExpressionResult::~LogicExpressionResult() {
        delete type;
    }
    
    LogicType* LogicExpressionResult::getType() {
        return type;
    }
    
    string LogicExpressionResult::toString() {
        return "\\result";
    }
    
    LogicType* LogicExpressionResult::returnType() {
        return type;
    }
    
    void LogicExpressionResult::checkTypes() {}
    
    void LogicExpressionResult::toWhy3(ostream &out, Why3Data &data) {
        if (data.calleeTheoryName) {
            out << data.calleeTheoryName << ".F.";
        }
        out << "ret_val";
    }
    
    bool LogicExpressionResult::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
