/*
 * expr_cast_int_real.cpp
 *
 *  Created on: Oct 7, 2016
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
    
    static const int classID = LOGIC_EXPR_CAST_INT_REAL;
    LogicExpressionIntToReal::LogicExpressionIntToReal(LogicExpression* expr, NodeSource* source) : LogicExpression(source), expr{expr} {
        id = classID;
    }
    LogicExpressionIntToReal::~LogicExpressionIntToReal() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionIntToReal::getExpr() {
        return expr;
    }
    
    string LogicExpressionIntToReal::toString() {
        return "(real) "+expr->toString();
    }
    
    static LogicTypeReal retType;
    LogicType* LogicExpressionIntToReal::returnType() {
        return &retType;
    }
    
    void LogicExpressionIntToReal::checkTypes() {
        expr->checkTypes();
        
        // expr must be an int
        if (!isa<LogicTypeInt>(expr->returnType())) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be an int", this);
        }
    }
    
    void LogicExpressionIntToReal::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("real.FromInt");
        out << "(from_int ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionIntToReal::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
