/*
 * expr_cast_real_int.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_REAL_INT;
    LogicExpressionRealToInt::LogicExpressionRealToInt(LogicExpression* expr, NodeSource* source) : LogicExpression(source), expr{expr} {
        id = classID;
    }
    LogicExpressionRealToInt::~LogicExpressionRealToInt() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionRealToInt::getExpr() {
        return expr;
    }
    
    string LogicExpressionRealToInt::toString() {
        return "(int) "+expr->toString();
    }
    
    static LogicTypeInt retType;
    LogicType* LogicExpressionRealToInt::returnType() {
        return &retType;
    }
    
    void LogicExpressionRealToInt::checkTypes() {
        expr->checkTypes();
        
        // expr must be a real
        if (!isa<LogicTypeReal>(expr->returnType())) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be a real", this);
        }
    }
    
    void LogicExpressionRealToInt::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("CustomTruncate");
        out << "(truncate ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionRealToInt::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
