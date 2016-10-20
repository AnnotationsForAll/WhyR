/*
 * expr_quant.cpp
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
    
    static const int classID = LOGIC_EXPR_QUANT;
    LogicExpressionQuantifier::LogicExpressionQuantifier(bool forall, list<LogicLocal*>* locals, LogicExpression* expr, NodeSource* source) : LogicExpression(source), forall{forall}, locals{locals}, expr{expr} {
        id = classID;
        
        // if locals is empty, a quantifier is nonsensical
        if (locals->empty()) {
            throw syntax_exception("Quantifiers must have at least one binder", this);
        }
    }
    LogicExpressionQuantifier::~LogicExpressionQuantifier() {}
    
    static string getQuantName(bool forall) {
        if (forall) {
            return "forall";
        } else {
            return "exists";
        }
    }
    
    bool LogicExpressionQuantifier::isForall() {
        return forall;
    }
    
    list<LogicLocal*>* LogicExpressionQuantifier::getLocals() {
        return locals;
    }
    
    LogicExpression* LogicExpressionQuantifier::getExpr() {
        return expr;
    }
    
    string LogicExpressionQuantifier::toString() {
        return getQuantName(forall) + "; " + expr->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionQuantifier::returnType() {
        return &retType;
    }
    
    void LogicExpressionQuantifier::checkTypes() {
        expr->checkTypes();
        
        // type of expr has to be boolean
        if (!expr->returnType()->equals(&retType)) {
            throw type_exception(("Operator '"+ getQuantName(forall) +"' undefined on type '" + expr->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionQuantifier::toWhy3(ostream &out, Why3Data &data) {
        for (list<LogicLocal*>::iterator ii = locals->begin(); ii != locals->end(); ii++) {
            out << getQuantName(forall) << " " << getWhy3LocalName(*ii) << ":";
            (*ii)->type->toWhy3(out, data);
            out << ". ";
        }
        expr->toWhy3(out, data);
    }
    
    bool LogicExpressionQuantifier::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
