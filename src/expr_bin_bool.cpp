/*
 * expr_bool_const.cpp
 *
 *  Created on: Sep 16, 2016
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
    
    static const int classID = LOGIC_EXPR_BIN_BOOL;
    LogicExpressionBinaryBoolean::LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::BinaryBooleanOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), op{op}, lhs{lhs}, rhs{rhs} {
        id = classID;
    }
    LogicExpressionBinaryBoolean::~LogicExpressionBinaryBoolean() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpressionBinaryBoolean::BinaryBooleanOp LogicExpressionBinaryBoolean::getOp() {
        return op;
    }
    
    LogicExpression* LogicExpressionBinaryBoolean::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionBinaryBoolean::getRight() {
        return rhs;
    }
    
    static string opString(LogicExpressionBinaryBoolean::BinaryBooleanOp op) {
        switch (op) {
            case LogicExpressionBinaryBoolean::OP_AND: return " /\\ ";
            case LogicExpressionBinaryBoolean::OP_OR: return " \\/ ";
            case LogicExpressionBinaryBoolean::OP_IMPLIES: return " -> ";
            case LogicExpressionBinaryBoolean::OP_BIDIR_IMPLIES: return " <-> ";
            default: throw whyr_exception(("internal error: Unknown LogicExpressionBinaryBoolean opcode "+to_string(op)));
        }
    }
    
    string LogicExpressionBinaryBoolean::toString() {
        return lhs->toString() + opString(op) + rhs->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBinaryBoolean::returnType() {
        return &retType;
    }
    
    void LogicExpressionBinaryBoolean::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        
        // both arguments must be boolean
        if (!LogicType::commonType(lhs->returnType(), &retType) || !LogicType::commonType(rhs->returnType(), &retType)) {
            throw type_exception(("Operator" + opString(op) + "undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBinaryBoolean::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        lhs->toWhy3(out, data);
        out << opString(op);
        rhs->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionBinaryBoolean::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
