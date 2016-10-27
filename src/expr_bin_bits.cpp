/*
 * expr_bin_bits.cpp
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
    
    static const int classID = LOGIC_EXPR_BIN_BITS;
    LogicExpressionBinaryBits::LogicExpressionBinaryBits(LogicExpressionBinaryBits::BinaryBitsOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryBits::~LogicExpressionBinaryBits() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryBits::BinaryBitsOp LogicExpressionBinaryBits::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryBits::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryBits::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryBits::toString() {
        switch (op) {
            case LogicExpressionBinaryBits::OP_AND: return lhs->toString() + " & " + rhs->toString();
            case LogicExpressionBinaryBits::OP_OR: return lhs->toString() + " | " + rhs->toString();
            case LogicExpressionBinaryBits::OP_SDIV: return lhs->toString() + " sdiv " + rhs->toString();
            case LogicExpressionBinaryBits::OP_SMOD: return lhs->toString() + " smod " + rhs->toString();
            case LogicExpressionBinaryBits::OP_SREM: return lhs->toString() + " srem " + rhs->toString();
            case LogicExpressionBinaryBits::OP_UDIV: return lhs->toString() + " udiv " + rhs->toString();
            case LogicExpressionBinaryBits::OP_UMOD: return lhs->toString() + " umod " + rhs->toString();
            case LogicExpressionBinaryBits::OP_UREM: return lhs->toString() + " urem " + rhs->toString();
            case LogicExpressionBinaryBits::OP_XOR: return lhs->toString() + " ^ " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryBits opcode "+to_string(op)), this);
        }
    }
    
    LogicType* LogicExpressionBinaryBits::returnType() {
        return lhs->returnType();
    }
    
    void LogicExpressionBinaryBits::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // the types of operands need to be equal
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Expression '" + toString() + "' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'").c_str(), this);
        }
        
        // The operands need to be LLVM ints, or LLVM int vectors
        if (!isa<LogicTypeLLVM>(returnType()) || !cast<LogicTypeLLVM>(returnType())->getType()->isIntOrIntVectorTy()) {
            throw type_exception(("Expression '" + toString() + "' undefined for non-LLVM-int type '" + returnType()->toString() + "'").c_str(), this);
        }
    }
    
    void LogicExpressionBinaryBits::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        out << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".";
        switch (op) {
            case LogicExpressionBinaryBits::OP_AND: {
                out << "bw_and ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_OR: {
                out << "bw_or ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_SDIV: {
                out << "sdiv ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_SMOD: {
                // FIXME: remainder and modulus are slightly different
                out << "srem ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_SREM: {
                out << "srem ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_UDIV: {
                out << "udiv ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_UMOD: {
                out << "urem ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_UREM: {
                out << "urem ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryBits::OP_XOR: {
                out << "bw_xor ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryBits opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryBits::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
