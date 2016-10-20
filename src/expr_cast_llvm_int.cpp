/*
 * expr_cast_llvm_int.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_LLVM_INT;
    LogicExpressionLLVMIntToLogicInt::LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::LLVMIntToLogicIntOp op, LogicExpression* expr, NodeSource* source) : LogicExpression(source), op{op}, expr{expr} {
        id = classID;
    }
    LogicExpressionLLVMIntToLogicInt::~LogicExpressionLLVMIntToLogicInt() {
        delete expr;
    }
    
    LogicExpressionLLVMIntToLogicInt::LLVMIntToLogicIntOp LogicExpressionLLVMIntToLogicInt::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionLLVMIntToLogicInt::getExpr() {
        return expr;
    }
    
    string LogicExpressionLLVMIntToLogicInt::toString() {
        switch (op) {
            case LogicExpressionLLVMIntToLogicInt::OP_ZEXT: return returnType()->toString()+" zext "+expr->toString();
            case LogicExpressionLLVMIntToLogicInt::OP_SEXT: return returnType()->toString()+" sext "+expr->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionLLVMIntToLogicInt opcode "+to_string(op)), this);
        }
    }
    
    static LogicTypeInt retType;
    LogicType* LogicExpressionLLVMIntToLogicInt::returnType() {
        return &retType;
    }
    
    void LogicExpressionLLVMIntToLogicInt::checkTypes() {
        expr->checkTypes();
        
        // expr must be an LLVM integer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be an LLVM integer type", this);
        }
    }
    
    void LogicExpressionLLVMIntToLogicInt::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        switch (op) {
            case LogicExpressionLLVMIntToLogicInt::OP_ZEXT: {
                out << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_uint ";
                expr->toWhy3(out, data);
                break;
            }
            case LogicExpressionLLVMIntToLogicInt::OP_SEXT: {
                out << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_int ";
                expr->toWhy3(out, data);
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionLLVMIntToLogicInt opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionLLVMIntToLogicInt::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
