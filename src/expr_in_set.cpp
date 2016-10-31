/*
 * expr_in_set.cpp
 *
 *  Created on: Oct 31, 2016
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
    
    static const int classID = LOGIC_EXPR_IN_SET;
    LogicExpressionInSet::LogicExpressionInSet(LogicExpression* setExpr, LogicExpression* itemExpr, NodeSource* source) : LogicExpression(source), setExpr{setExpr}, itemExpr{itemExpr} {
        id = classID;
    }
    LogicExpressionInSet::~LogicExpressionInSet() {
        delete setExpr;
        delete itemExpr;
    }
    
    LogicExpression* LogicExpressionInSet::getSetExpr() {
        return setExpr;
    }
    
    LogicExpression* LogicExpressionInSet::getItemExpr() {
        return itemExpr;
    }
    
    string LogicExpressionInSet::toString() {
        return itemExpr->toString() + " in " + setExpr->toString();
    }
    
    LogicTypeBool retType;
    LogicType* LogicExpressionInSet::returnType() {
        return &retType;
    }
    
    void LogicExpressionInSet::checkTypes() {
        setExpr->checkTypes();
        itemExpr->checkTypes();
        
        // setExpr must be a set
        // itemExpr must be the element type of the set
    }
    
    void LogicExpressionInSet::toWhy3(ostream &out, Why3Data &data) {
        
    }
    
    bool LogicExpressionInSet::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
