/*
 * expr_cast_int_llvm.cpp
 *
 *  Created on: Oct 5, 2016
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
    
    static const int classID = LOGIC_EXPR_CAST_INT_LLVM;
    LogicExpressionLogicIntToLLVMInt::LogicExpressionLogicIntToLLVMInt(LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionLogicIntToLLVMInt::~LogicExpressionLogicIntToLLVMInt() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionLogicIntToLLVMInt::getExpr() {
        return expr;
    }
    
    string LogicExpressionLogicIntToLLVMInt::toString() {
        return "("+retType->toString()+") "+expr->toString();
    }
    
    LogicType* LogicExpressionLogicIntToLLVMInt::returnType() {
        return retType;
    }
    
    void LogicExpressionLogicIntToLLVMInt::checkTypes() {
        expr->checkTypes();
        
        // retType must be an LLVM integer
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be an LLVM integer type", this);
        }
        
        // expr must be an int
        if (!isa<LogicTypeInt>(expr->returnType())) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be a logical integer type", this);
        }
    }
    
    void LogicExpressionLogicIntToLLVMInt::toWhy3(ostream &out, Why3Data &data) {
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".of_int ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionLogicIntToLLVMInt::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
