/*
 * expr_old.cpp
 *
 *  Created on: Nov 1, 2016
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
    
    static const int classID = LOGIC_EXPR_OLD;
    LogicExpressionOld::LogicExpressionOld(LogicExpression* expr, NodeSource* source) : LogicExpression(source), expr{expr} {
        id = classID;
    }
    LogicExpressionOld::~LogicExpressionOld() {}
    
    LogicExpression* LogicExpressionOld::getExpr() {
        return expr;
    }
    
    string LogicExpressionOld::toString() {
        return "old " + expr->toString();
    }
    
    LogicType* LogicExpressionOld::returnType() {
        return expr->returnType();
    }
    
    void LogicExpressionOld::checkTypes() {
        expr->checkTypes();
    }
    
    void LogicExpressionOld::toWhy3(ostream &out, Why3Data &data) {
        string oldState = data.statepoint;
        if (data.calleeTheoryName) {
            data.statepoint = string(data.calleeTheoryName) + ".F.entry_state";
        } else {
            data.statepoint = "entry_state";
        }
        
        expr->toWhy3(out, data);
        
        data.statepoint = oldState;
    }
    
    bool LogicExpressionOld::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
