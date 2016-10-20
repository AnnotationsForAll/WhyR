/*
 * expr_let.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/exception.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_LET;
    LogicExpressionLet::LogicExpressionLet(list<pair<LogicLocal*, LogicExpression*>*>* locals, LogicExpression* expr, NodeSource* source) : LogicExpression(source), locals{locals}, expr{expr} {
        id = classID;
        
        // if locals is empty, a let is nonsensical
        if (locals->empty()) {
            throw syntax_exception("Let statements must have at least one binder", this);
        }
    }
    LogicExpressionLet::~LogicExpressionLet() {}
    
    list<pair<LogicLocal*, LogicExpression*>*>* LogicExpressionLet::getLocals() {
        return locals;
    }
    
    LogicExpression* LogicExpressionLet::getExpr() {
        return expr;
    }
    
    string LogicExpressionLet::toString() {
        return "let: " + expr->toString();
    }
    
    LogicType* LogicExpressionLet::returnType() {
        return expr->returnType();
    }
    
    void LogicExpressionLet::checkTypes() {
        expr->checkTypes();
        
        // all locals must match thier expression types
        for (list<pair<LogicLocal*, LogicExpression*>*>::iterator ii = locals->begin(); ii != locals->end(); ii++) {
            if (!(*ii)->first->type->equals((*ii)->second->returnType())) {
                throw type_exception(("Local variable '" + (*ii)->first->name + "' is of type '" + (*ii)->first->type->toString() + "'; got an expression of type '" + (*ii)->second->returnType()->toString() + "'"), this);
            }
        }
    }
    
    void LogicExpressionLet::toWhy3(ostream &out, Why3Data &data) {
        for (list<pair<LogicLocal*, LogicExpression*>*>::iterator ii = locals->begin(); ii != locals->end(); ii++) {
            out << "let " << getWhy3LocalName((*ii)->first) << " = ";
            (*ii)->second->toWhy3(out, data);
            out << " in ";
        }
        expr->toWhy3(out, data);
    }
    
    bool LogicExpressionLet::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
