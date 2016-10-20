/*
 * expr_cast_real_float.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_REAL_FLOAT;
    LogicExpressionRealToFloat::LogicExpressionRealToFloat(LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionRealToFloat::~LogicExpressionRealToFloat() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionRealToFloat::getExpr() {
        return expr;
    }
    
    string LogicExpressionRealToFloat::toString() {
        return "("+retType->toString()+") "+expr->toString();
    }
    
    LogicType* LogicExpressionRealToFloat::returnType() {
        return retType;
    }
    
    void LogicExpressionRealToFloat::checkTypes() {
        expr->checkTypes();
        
        // retType must be an LLVM floating type
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isFloatingPointTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be an LLVM floating type", this);
        }
        
        // expr must be an real
        if (!isa<LogicTypeReal>(expr->returnType())) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be a logical real type", this);
        }
    }
    
    void LogicExpressionRealToFloat::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("floating_point.Rounding");
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".of_real Rounding.NearestTiesToEven ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionRealToFloat::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
