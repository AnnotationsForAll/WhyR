/*
 * expr_fresh.cpp
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
    
    static const int classID = LOGIC_EXPR_FRESH;
    LogicExpressionFresh::LogicExpressionFresh(bool before, LogicExpression* expr, NodeSource* source) : LogicExpression(source), before{before}, expr{expr} {
        id = classID;
    }
    LogicExpressionFresh::~LogicExpressionFresh() {}
    
    LogicExpression* LogicExpressionFresh::getExpr() {
        return expr;
    }
    
    string LogicExpressionFresh::toString() {
        return string("fresh ") + (before ? "before" : "after") + " " + expr->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionFresh::returnType() {
        return &retType;
    }
    
    void LogicExpressionFresh::checkTypes() {
        expr->checkTypes();
        
        // argument must be a pointer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isPointerTy()) {
            throw type_exception("Argument to 'fresh' must be a pointer, got an argument of type '" + expr->returnType()->toString() + "'", this);
        }
    }
    
    void LogicExpressionFresh::toWhy3(ostream &out, Why3Data &data) {
        data.info->usesAlloc = true;
        
        out << "(allocated_" << (before ? "before" : "after") << " " << data.statepoint << " ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionFresh::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
