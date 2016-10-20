/*
 * expr_bin_comp_float.cpp
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
    
    static const int classID = LOGIC_EXPR_BIN_COMP_FLOAT;
    LogicExpressionBinaryCompareFloat::LogicExpressionBinaryCompareFloat(LogicExpressionBinaryCompareFloat::BinaryCompareFloatOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryCompareFloat::~LogicExpressionBinaryCompareFloat() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryCompareFloat::BinaryCompareFloatOp LogicExpressionBinaryCompareFloat::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryCompareFloat::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryCompareFloat::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryCompareFloat::toString() {
        switch (op) {
            case LogicExpressionBinaryCompareFloat::OP_OEQ: return lhs->toString() + " foeq " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_OGT: return lhs->toString() + " fogt " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_OGE: return lhs->toString() + " foge " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_OLT: return lhs->toString() + " folt " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_OLE: return lhs->toString() + " fole " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_ONE: return lhs->toString() + " fone " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_ORD: return lhs->toString() + " ford " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_UEQ: return lhs->toString() + " fueq " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_UGT: return lhs->toString() + " fugt " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_UGE: return lhs->toString() + " fuge " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_ULT: return lhs->toString() + " fult " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_ULE: return lhs->toString() + " fule " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_UNE: return lhs->toString() + " fune " + rhs->toString();
            case LogicExpressionBinaryCompareFloat::OP_UNO: return lhs->toString() + " funo " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompareFloat opcode "+to_string(op)), this);
        }
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBinaryCompareFloat::returnType() {
        return &retType;
    }
    
    void LogicExpressionBinaryCompareFloat::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // the types of operands need to be equal
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Expression '" + toString() + "' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
        
        // The operands need to be LLVM floats
        if (!isa<LogicTypeLLVM>(lhs->returnType()) || !cast<LogicTypeLLVM>(lhs->returnType())->getType()->isFloatingPointTy()) {
            throw type_exception(("Expression '" + toString() + "' undefined for type '" + lhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBinaryCompareFloat::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        out << getWhy3TheoryName(cast<LogicTypeLLVM>(lhs->returnType())->getType()) << ".";
        
        switch (op) {
            case LogicExpressionBinaryCompareFloat::OP_OEQ: {
                out << "oeq ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_OGT: {
                out << "ogt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_OGE: {
                out << "oge ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_OLT: {
                out << "olt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_OLE: {
                out << "ole ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_ONE: {
                out << "one ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_ORD: {
                out << "ord ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_UEQ: {
                out << "ueq ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_UGT: {
                out << "ugt ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_UGE: {
                out << "uge ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_ULT: {
                out << "ult ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_ULE: {
                out << "ule ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_UNE: {
                out << "une ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            case LogicExpressionBinaryCompareFloat::OP_UNO: {
                out << "uno ";
                lhs->toWhy3(out, data);
                out << " ";
                rhs->toWhy3(out, data);
                break;
            }
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompareFloat opcode "+to_string(op)), this);
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryCompareFloat::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
