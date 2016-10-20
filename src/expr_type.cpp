/*
 * expr_type.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_TYPE;
    LogicExpressionConstantType::LogicExpressionConstantType(LogicTypeType* type, NodeSource* source) : LogicExpression(source), type{type} {
        id = classID;
    }
    LogicExpressionConstantType::~LogicExpressionConstantType() {
        delete type;
    }
    
    LogicTypeType* LogicExpressionConstantType::getType() {
        return type;
    }
    
    string LogicExpressionConstantType::toString() {
        return "type " + type->getType()->toString();
    }
    
    LogicType* LogicExpressionConstantType::returnType() {
        return type;
    }
    
    void LogicExpressionConstantType::checkTypes() {}
    
    bool LogicExpressionConstantType::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
