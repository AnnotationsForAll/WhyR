/*
 * expr_special_llvm_const.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>

#include <sstream>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_SPECIAL_LLVM_CONST;
    LogicExpressionSpecialLLVMConstant::LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::SpecialLLVMConstOp op, LogicType* retType, NodeSource* source) : LogicExpression(source), op{op}, retType{retType} {
        id = classID;
    }
    LogicExpressionSpecialLLVMConstant::~LogicExpressionSpecialLLVMConstant() {
        delete retType;
    }
    
    LogicExpressionSpecialLLVMConstant::SpecialLLVMConstOp LogicExpressionSpecialLLVMConstant::getOp() {
        return op;
    }
    
    string LogicExpressionSpecialLLVMConstant::toString() {
        switch (op) {
            case LogicExpressionSpecialLLVMConstant::OP_MAXINT: return "(" + retType->toString() + " sext) max";
            case LogicExpressionSpecialLLVMConstant::OP_MININT: return "(" + retType->toString() + " sext) min";
            case LogicExpressionSpecialLLVMConstant::OP_MAXUINT: return "(" + retType->toString() + " zext) max";
            case LogicExpressionSpecialLLVMConstant::OP_MINUINT: return "(" + retType->toString() + " zext) min";
            default: throw whyr_exception(("internal error: Unknown LogicExpressionSpecialLLVMConstant opcode "+to_string(op)), this);
        }
    }
    
    LogicType* LogicExpressionSpecialLLVMConstant::returnType() {
        return retType;
    }
    
    void LogicExpressionSpecialLLVMConstant::checkTypes() {
        switch (op) {
            case LogicExpressionSpecialLLVMConstant::OP_MAXINT:
            case LogicExpressionSpecialLLVMConstant::OP_MININT:
            case LogicExpressionSpecialLLVMConstant::OP_MAXUINT:
            case LogicExpressionSpecialLLVMConstant::OP_MINUINT: {
                // the return type needs to be an LLVM int
                if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isIntegerTy()) {
                    throw type_exception(("Expression '" + toString() + "' undefined for non-LLVM-int type '" + retType->toString() + "'"), this);
                }
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionSpecialLLVMConstant opcode "+to_string(op)), this);
        }
    }
    
    void LogicExpressionSpecialLLVMConstant::toWhy3(ostream &out, Why3Data &data) {
        ostringstream const_stream;
        
        switch (op) {
            case LogicExpressionSpecialLLVMConstant::OP_MAXINT: {
                const_stream << "0b1";
                for (unsigned i = 1; i < cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth(); i++) {
                    const_stream << "0";
                }
                break;
            }
            case LogicExpressionSpecialLLVMConstant::OP_MAXUINT: {
                const_stream << "0b";
                for (unsigned i = 0; i < cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth(); i++) {
                    const_stream << "1";
                }
                break;
            }
            case LogicExpressionSpecialLLVMConstant::OP_MININT: {
                const_stream << "(- 0b1";
                for (unsigned i = 0; i < cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth(); i++) {
                    const_stream << "0";
                }
                const_stream << ")";
                break;
            }
            case LogicExpressionSpecialLLVMConstant::OP_MINUINT: {
                const_stream << "0";
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionSpecialLLVMConstant opcode "+to_string(op)), this);
        }
        
        addLLVMIntConstant(out, data.module, cast<LogicTypeLLVM>(retType)->getType(), const_stream.str());
    }
    
    bool LogicExpressionSpecialLLVMConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
