/*
 * expr_bin_math.cpp
 *
 *  Created on: Sep 30, 2016
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
    
    static const int classID = LOGIC_EXPR_BIN_MATH;
    LogicExpressionBinaryMath::LogicExpressionBinaryMath(LogicExpressionBinaryMath::BinaryMathOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryMath::~LogicExpressionBinaryMath() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryMath::BinaryMathOp LogicExpressionBinaryMath::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryMath::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryMath::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryMath::toString() {
        switch (op) {
            case LogicExpressionBinaryMath::OP_ADD: return lhs->toString() + " + " + rhs->toString();
            case LogicExpressionBinaryMath::OP_SUB: return lhs->toString() + " - " + rhs->toString();
            case LogicExpressionBinaryMath::OP_MUL: return lhs->toString() + " * " + rhs->toString();
            case LogicExpressionBinaryMath::OP_DIV: return lhs->toString() + " / " + rhs->toString();
            case LogicExpressionBinaryMath::OP_MOD: return lhs->toString() + " mod " + rhs->toString();
            case LogicExpressionBinaryMath::OP_REM: return lhs->toString() + " rem " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryMath opcode "+to_string(op)), this);
        }
    }
    
    LogicType* LogicExpressionBinaryMath::returnType() {
        return lhs->returnType();
    }
    
    void LogicExpressionBinaryMath::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // the types of operands need to be equal
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Expression '" + toString() + "' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
        
        // The operands need to be numeric in some manner
        switch (returnType()->id) {
            case LOGIC_TYPE_LLVM: {
                // fail only if LLVM type is non-numeric
                Type* ty = cast<LogicTypeLLVM>(returnType())->getType();
                if (!(ty->isIntOrIntVectorTy() || ty->isFPOrFPVectorTy())) {
                    goto non_numeric_type;
                }
                break;
            }
            case LOGIC_TYPE_INT:
            case LOGIC_TYPE_REAL: {
                // always valid
                break;
            }
            default: {
                // not a numeric type; throw an error
                non_numeric_type:
                throw type_exception(("Expression '" + toString() + "' undefined for non-numeric type '" + returnType()->toString() + "'"), this);
            }
        }
        
        // if this is an LLVM int type operator, DIV, MOD, and REM are not defined operators
        if (isa<LogicTypeLLVM>(returnType()) && cast<LogicTypeLLVM>(returnType())->getType()->isIntOrIntVectorTy()) {
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD:
                case LogicExpressionBinaryMath::OP_SUB:
                case LogicExpressionBinaryMath::OP_MUL: {
                    break;
                }
                default: {
                    throw type_exception(("Expression '" + toString() + "' undefined for LLVM type '" + returnType()->toString() + "'"), this);
                }
            }
        }
        
        // if this is an real type operator, MOD and REM are not defined operators
        if (isa<LogicTypeReal>(returnType())) {
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD:
                case LogicExpressionBinaryMath::OP_SUB:
                case LogicExpressionBinaryMath::OP_MUL:
                case LogicExpressionBinaryMath::OP_DIV: {
                    break;
                }
                default: {
                    throw type_exception(("Expression '" + toString() + "' undefined for real type"), this);
                }
            }
        }
    }
    
    void LogicExpressionBinaryMath::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        if (isa<LogicTypeInt>(returnType())) {
            // is an integer type
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD: {
                    lhs->toWhy3(out, data);
                    out << " + ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_SUB: {
                    lhs->toWhy3(out, data);
                    out << " - ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MUL: {
                    lhs->toWhy3(out, data);
                    out << " * ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_DIV: {
                    data.importsNeeded.insert("int.ComputerDivision");
                    out << "div ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MOD: {
                    data.importsNeeded.insert("int.ComputerDivision");
                    out << "mod ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_REM: {
                    data.importsNeeded.insert("int.ComputerDivision");
                    out << "let op_result = (mod ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    out << ") in if ";
                    lhs->toWhy3(out, data);
                    out << " >= 0 <-> ";
                    rhs->toWhy3(out, data);
                    out << " >= 0 then op_result else (- op_result)";
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryMath opcode "+to_string(op)), this);
            }
        } else if (isa<LogicTypeReal>(returnType())) {
            // is a real type
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD: {
                    lhs->toWhy3(out, data);
                    out << " +. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_SUB: {
                    lhs->toWhy3(out, data);
                    out << " -. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MUL: {
                    lhs->toWhy3(out, data);
                    out << " *. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_DIV: {
                    lhs->toWhy3(out, data);
                    out << " /. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryMath opcode "+to_string(op)), this);
            }
        } else if (isa<LogicTypeLLVM>(returnType()) && cast<LogicTypeLLVM>(returnType())->getType()->isIntOrIntVectorTy()) {
            // is an integer LLVM type
            out << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".";
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD: {
                    out << "add ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_SUB: {
                    out << "sub ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MUL: {
                    out << "mul ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryMath opcode "+to_string(op)), this);
            }
        } else if (isa<LogicTypeLLVM>(returnType()) && cast<LogicTypeLLVM>(returnType())->getType()->isFPOrFPVectorTy()) {
            // is a float type
            data.importsNeeded.insert("floating_point.Rounding");
            out << getWhy3TheoryName(cast<LogicTypeLLVM>(returnType())->getType()) << ".";
            switch (op) {
                case LogicExpressionBinaryMath::OP_ADD: {
                    out << "fadd Rounding.NearestTiesToEven ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_SUB: {
                    out << "fsub Rounding.NearestTiesToEven ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MUL: {
                    out << "fmul Rounding.NearestTiesToEven ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_DIV: {
                    out << "fdiv Rounding.NearestTiesToEven ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryMath::OP_MOD: {
                    // FIXME
                    
                    if (data.module->getSettings()) {
                        data.module->getSettings()->warnings.push_back(whyr_warning("mod currently not supported on floats; using rem instead", this));
                    }
                }
                case LogicExpressionBinaryMath::OP_REM: {
                    out << "frem Rounding.NearestTiesToEven ";
                    lhs->toWhy3(out, data);
                    out << " ";
                    rhs->toWhy3(out, data);
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryMath opcode "+to_string(op)), this);
            }
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryMath::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
