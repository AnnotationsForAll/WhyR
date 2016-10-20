/*
 * expr_cast_llvm_llvm.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_LLVM_LLVM;
    LogicExpressionLLVMIntToLLVMInt::LogicExpressionLLVMIntToLLVMInt(LogicExpressionLLVMIntToLLVMInt::LLVMIntToLLVMIntOp op, LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), op{op}, expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionLLVMIntToLLVMInt::~LogicExpressionLLVMIntToLLVMInt() {
        delete expr;
    }
    
    LogicExpressionLLVMIntToLLVMInt::LLVMIntToLLVMIntOp LogicExpressionLLVMIntToLLVMInt::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionLLVMIntToLLVMInt::getExpr() {
        return expr;
    }
    
    string LogicExpressionLLVMIntToLLVMInt::toString() {
        switch (op) {
            case LogicExpressionLLVMIntToLLVMInt::OP_TRUNC: return "("+retType->toString()+") "+expr->toString();
            case LogicExpressionLLVMIntToLLVMInt::OP_ZEXT: return retType->toString()+" zext "+expr->toString();
            case LogicExpressionLLVMIntToLLVMInt::OP_SEXT: return retType->toString()+" sext "+expr->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionLLVMIntToLLVMInt opcode "+to_string(op)), this);
        }
    }
    
    LogicType* LogicExpressionLLVMIntToLLVMInt::returnType() {
        return retType;
    }
    
    void LogicExpressionLLVMIntToLLVMInt::checkTypes() {
        expr->checkTypes();
        
        // retType must be an LLVM integer
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be an LLVM integer type", this);
        }
        
        // expr must be an LLVM integer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be an LLVM integer type", this);
        }
        
        // sizes must be correct
        if (op == LogicExpressionLLVMIntToLLVMInt::OP_TRUNC) {
            // if TRUNC, retType < expr
            if (cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth() >= cast<LogicTypeLLVM>(expr->returnType())->getType()->getIntegerBitWidth()) {
                throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"' to type '"+retType->toString()+"'; sizes are not correct for truncation", this);
            }
        } else {
            // else, retType > expr
            if (cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth() <= cast<LogicTypeLLVM>(expr->returnType())->getType()->getIntegerBitWidth()) {
                throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"' to type '"+retType->toString()+"'; sizes are not correct for extension", this);
            }
        }
    }
    
    void LogicExpressionLLVMIntToLLVMInt::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        switch (op) {
            case LogicExpressionLLVMIntToLLVMInt::OP_TRUNC: {
                data.importsNeeded.insert("int.ComputerDivision");
                
                out <<  getWhy3TheoryName(cast<LogicTypeLLVM>(retType)->getType()) << ".of_int (mod (" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_uint ";
                expr->toWhy3(out, data);
                out << ") 0b1";
                for (unsigned i = 0; i < cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth(); i++) {
                    out << "0";
                }
                out << ")";
                break;
            }
            case LogicExpressionLLVMIntToLLVMInt::OP_ZEXT: {
                out << getWhy3TheoryName(cast<LogicTypeLLVM>(retType)->getType()) << ".of_int (" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_uint ";
                expr->toWhy3(out, data);
                out << ")";
                break;
            }
            case LogicExpressionLLVMIntToLLVMInt::OP_SEXT: {
                out << getWhy3TheoryName(cast<LogicTypeLLVM>(retType)->getType()) << ".of_int (" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_int ";
                expr->toWhy3(out, data);
                out << ")";
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionLLVMIntToLLVMInt opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionLLVMIntToLLVMInt::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
