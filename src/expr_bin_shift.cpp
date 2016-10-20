/*
 * expr_bin_shift.cpp
 *
 *  Created on: Oct 4, 2016
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
    
    static const int classID = LOGIC_EXPR_BIN_SHIFT;
    LogicExpressionBinaryShift::LogicExpressionBinaryShift(LogicExpressionBinaryShift::BinaryShiftOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryShift::~LogicExpressionBinaryShift() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryShift::BinaryShiftOp LogicExpressionBinaryShift::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryShift::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryShift::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryShift::toString() {
        switch (op) {
            case LogicExpressionBinaryShift::OP_ASHR: return lhs->toString() + " ashr " + rhs->toString();
            case LogicExpressionBinaryShift::OP_LSHL: return lhs->toString() + " shl " + rhs->toString();
            case LogicExpressionBinaryShift::OP_LSHR: return lhs->toString() + " lshr " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryShift opcode "+to_string(op)), this);
        }
    }
    
    LogicType* LogicExpressionBinaryShift::returnType() {
        return lhs->returnType();
    }
    
    void LogicExpressionBinaryShift::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // Operand 1 needs to be an LLVM int
        if (isa<LogicTypeLLVM>(lhs->returnType())) {
            if (!cast<LogicTypeLLVM>(lhs->returnType())->getType()->isIntegerTy()) {
                throw type_exception(("Operand 1 of expression '" + toString() + "' undefined for non-LLVM-int type '" + lhs->returnType()->toString() + "'"), this);
            }
        } else {
            throw type_exception(("Operand 1 of expression '" + toString() + "' undefined for non-LLVM-int type '" + lhs->returnType()->toString() + "'"), this);
        }
        
        // Operand 2 needs to be a logic int
        if (!isa<LogicTypeInt>(rhs->returnType())) {
            throw type_exception(("Operand 2 of expression '" + toString() + "' undefined for non-int type '" + rhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBinaryShift::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        out << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".";
        switch (op) {
            case LogicExpressionBinaryShift::OP_ASHR: {
                out << "asr ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryShift::OP_LSHL: {
                out << "lsl ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryShift::OP_LSHR: {
                out << "lsr ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryShift opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryShift::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
