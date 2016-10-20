/*
 * expr_eq.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_EQ;
    LogicExpressionEquals::LogicExpressionEquals(LogicExpression* lhs, LogicExpression* rhs, bool negated, NodeSource* source) : LogicExpression(source), lhs{lhs}, rhs{rhs}, negated{negated} {
        id = classID;
    }
    
    LogicExpressionEquals::~LogicExpressionEquals() {
        delete lhs;
        delete rhs;
    }
    
    LogicExpression* LogicExpressionEquals::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionEquals::getRight() {
        return rhs;
    }
    
    bool LogicExpressionEquals::isNegated() {
        return negated;
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionEquals::returnType() {
        return &retType;
    }
    
    string LogicExpressionEquals::toString() {
        return "(" + lhs->toString() + (negated ? " != " : " == ") + rhs->toString() + ")";
    }
    
    void LogicExpressionEquals::checkTypes() {
        // both arguments must be of the exact same type, as this is what they Why3 "=" operator expects
        lhs->checkTypes();
        rhs->checkTypes();
        
        if (!lhs->returnType()->equals(rhs->returnType())) {
            throw type_exception(("Operator 'eq' undefined between types '" + lhs->returnType()->toString() + "' and '" + rhs->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionEquals::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        lhs->toWhy3(out, data);
        if (negated) {
            out << " <> ";
        } else {
            out << " = ";
        }
        rhs->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionEquals::classof(const LogicExpression* type) {
        return type->id == classID;
    }
}
