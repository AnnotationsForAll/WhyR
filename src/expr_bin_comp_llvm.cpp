/*
 * expr_bin_comp_llvm.cpp
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
    
    static const int classID = LOGIC_EXPR_BIN_COMP_LLVM;
    LogicExpressionBinaryCompareLLVM::LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::BinaryCompareLLVMOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryCompareLLVM::~LogicExpressionBinaryCompareLLVM() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryCompareLLVM::BinaryCompareLLVMOp LogicExpressionBinaryCompareLLVM::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryCompareLLVM::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryCompareLLVM::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryCompareLLVM::toString() {
        switch (op) {
            case LogicExpressionBinaryCompareLLVM::OP_SGE: return lhs->toString() + " sge " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_SGT: return lhs->toString() + " sgt " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_SLE: return lhs->toString() + " sle " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_SLT: return lhs->toString() + " slt " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_UGE: return lhs->toString() + " uge " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_UGT: return lhs->toString() + " ugt " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_ULE: return lhs->toString() + " ule " + rhs->toString();
            case LogicExpressionBinaryCompareLLVM::OP_ULT: return lhs->toString() + " ult " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompareLLVM opcode "+to_string(op)), this);
        }
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBinaryCompareLLVM::returnType() {
        return &retType;
    }
    
    void LogicExpressionBinaryCompareLLVM::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // the types of operands need to be equal
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Expression '" + toString() + "' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
        
        // The operands need to be LLVM ints
        if (isa<LogicTypeLLVM>(lhs->returnType())) {
            if (!cast<LogicTypeLLVM>(lhs->returnType())->getType()->isIntegerTy()) {
                throw type_exception(("Expression '" + toString() + "' undefined for non-LLVM-int type '" + lhs->returnType()->toString() + "'"), this);
            }
        } else {
            throw type_exception(("Expression '" + toString() + "' undefined for non-LLVM-int type '" + lhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBinaryCompareLLVM::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        out << getWhy3TheoryName(cast<LogicTypeLLVM>(lhs->returnType())->getType()) << ".";
        switch (op) {
            case LogicExpressionBinaryCompareLLVM::OP_SGE: {
                out << "sge ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_SGT: {
                out << "sgt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_SLE: {
                out << "sle ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_SLT: {
                out << "slt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_UGE: {
                out << "uge ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_UGT: {
                out << "ugt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_ULE: {
                out << "ule ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareLLVM::OP_ULT: {
                out << "ult ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompareLLVM opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryCompareLLVM::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
