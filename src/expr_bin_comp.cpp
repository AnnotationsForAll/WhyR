/*
 * expr_bin_comp.cpp
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
    
    static const int classID = LOGIC_EXPR_BIN_COMP;
    LogicExpressionBinaryCompare::LogicExpressionBinaryCompare(LogicExpressionBinaryCompare::BinaryCompareOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryCompare::~LogicExpressionBinaryCompare() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryCompare::BinaryCompareOp LogicExpressionBinaryCompare::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryCompare::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryCompare::getRight() {
        return rhs;
    }
    
    string LogicExpressionBinaryCompare::toString() {
        switch (op) {
            case LogicExpressionBinaryCompare::OP_GE: return lhs->toString() + " >= " + rhs->toString();
            case LogicExpressionBinaryCompare::OP_GT: return lhs->toString() + " > " + rhs->toString();
            case LogicExpressionBinaryCompare::OP_LE: return lhs->toString() + " <= " + rhs->toString();
            case LogicExpressionBinaryCompare::OP_LT: return lhs->toString() + " < " + rhs->toString();
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompare opcode "+to_string(op)), this);
        }
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBinaryCompare::returnType() {
        return &retType;
    }
    
    void LogicExpressionBinaryCompare::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // the types of operands need to be equal
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Expression '" + toString() + "' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
        
        // The operands need to be logic ints or logic reals
        if (isa<LogicTypeInt>(lhs->returnType())) {
        } else if (isa<LogicTypeReal>(lhs->returnType())) {
        } else {
            throw type_exception(("Expression '" + toString() + "' undefined for type '" + lhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBinaryCompare::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        
        if (isa<LogicTypeInt>(lhs->returnType())) {
            // is an int type
            switch (op) {
                case LogicExpressionBinaryCompare::OP_GE: {
                    lhs->toWhy3(out, data);
                    out << " >= ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_GT: {
                    lhs->toWhy3(out, data);
                    out << " > ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_LE: {
                    lhs->toWhy3(out, data);
                    out << " <= ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_LT: {
                    lhs->toWhy3(out, data);
                    out << " < ";
                    rhs->toWhy3(out, data);
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompare opcode "+to_string(op)), this);
            }
        } else if (isa<LogicTypeReal>(lhs->returnType())) {
            // is a real type
            switch (op) {
                case LogicExpressionBinaryCompare::OP_GE: {
                    lhs->toWhy3(out, data);
                    out << " >=. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_GT: {
                    lhs->toWhy3(out, data);
                    out << " >. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_LE: {
                    lhs->toWhy3(out, data);
                    out << " <=. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                case LogicExpressionBinaryCompare::OP_LT: {
                    lhs->toWhy3(out, data);
                    out << " <. ";
                    rhs->toWhy3(out, data);
                    break;
                }
                default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryCompare opcode "+to_string(op)), this);
            }
        }
        
        out << ")";
    }
    
    bool LogicExpressionBinaryCompare::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
