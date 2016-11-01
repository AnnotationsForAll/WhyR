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
    LogicExpressionInSet::~LogicExpressionInSet() {}
    
    LogicExpression* LogicExpressionInSet::getSetExpr() {
        return setExpr;
    }
    
    LogicExpression* LogicExpressionInSet::getItemExpr() {
        return itemExpr;
    }
    
    string LogicExpressionInSet::toString() {
        return itemExpr->toString() + " in " + setExpr->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionInSet::returnType() {
        return &retType;
    }
    
    void LogicExpressionInSet::checkTypes() {
        setExpr->checkTypes();
        itemExpr->checkTypes();
        
        // setExpr must be a set
        if (!isa<LogicTypeSet>(setExpr->returnType())) {
            throw type_exception("Set argument of 'in' expected to be of type 'set<" + itemExpr->returnType()->toString() + ">'; got type '" + setExpr->returnType()->toString() + "'", this);
        }
        
        // itemExpr must be the element type of the set
        if (!itemExpr->returnType()->equals(cast<LogicTypeSet>(setExpr->returnType())->getType())) {
            throw type_exception("Element argument of 'in' expected to be of type '" + cast<LogicTypeSet>(setExpr->returnType())->getType()->toString() + "'; got type '" + itemExpr->returnType()->toString() + "'", this);
        }
    }
    
    void LogicExpressionInSet::toWhy3(ostream &out, Why3Data &data) {
        string theory;
        if (isa<LogicTypeLLVM>(itemExpr->returnType()) && cast<LogicTypeLLVM>(itemExpr->returnType())->getType()->isPointerTy()) {
            data.importsNeeded.insert("MemorySet");
            theory = "MemorySet";
        } else {
            data.importsNeeded.insert("set.Set");
            theory = "Set";
        }
        
        out << "(" << theory << ".mem (";
        itemExpr->toWhy3(out, data);
        out << ":";
        itemExpr->returnType()->toWhy3(out, data);
        out << ") ";
        setExpr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionInSet::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
