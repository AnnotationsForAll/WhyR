/*
 * expr_cast_float_real.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_FLOAT_REAL;
    LogicExpressionFloatToReal::LogicExpressionFloatToReal(LogicExpression* expr, NodeSource* source) : LogicExpression(source), expr{expr} {
        id = classID;
    }
    LogicExpressionFloatToReal::~LogicExpressionFloatToReal() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionFloatToReal::getExpr() {
        return expr;
    }
    
    string LogicExpressionFloatToReal::toString() {
        return "(real) "+expr->toString();
    }
    
    static LogicTypeReal retType;
    LogicType* LogicExpressionFloatToReal::returnType() {
        return &retType;
    }
    
    void LogicExpressionFloatToReal::checkTypes() {
        expr->checkTypes();
        
        // expr must be a float
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isFloatingPointTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be an LLVM floating type", this);
        }
    }
    
    void LogicExpressionFloatToReal::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("floating_point.Rounding");
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_real Rounding.NearestTiesToEven ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionFloatToReal::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
