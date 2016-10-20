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

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_BOOL_CONST;
    LogicExpressionBooleanConstant::LogicExpressionBooleanConstant(bool value, NodeSource* source) : LogicExpression(source), value{value} {
        id = classID;
    }
    LogicExpressionBooleanConstant::~LogicExpressionBooleanConstant() {}
    
    bool LogicExpressionBooleanConstant::getValue() {
        return value;
    }
    
    string LogicExpressionBooleanConstant::toString() {
        return value ? "\\true" : "\\false";
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBooleanConstant::returnType() {
        return &retType;
    }
    
    void LogicExpressionBooleanConstant::checkTypes() {}
    
    void LogicExpressionBooleanConstant::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("bool.Bool"); // add the import, so we can treat booleans as terms
        out << (value ? "true" : "false");
    }
    
    bool LogicExpressionBooleanConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
