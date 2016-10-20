/*
 * expr_neg.cpp
 *
 *  Created on: Oct 3, 2016
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
    
    static const int classID = LOGIC_EXPR_LOAD;
    LogicExpressionLoad::LogicExpressionLoad(LogicExpression* expr, NodeSource* source) : LogicExpression(source), expr{expr} {
        id = classID;
        
        // we have to type check in order to get the return value.
        if (!isa<LogicTypeLLVM>(expr->returnType())) {
            throw type_exception("Argument to 'load' must be a pointer; got an expression of type '" + expr->returnType()->toString() + "'", this);
        }
        Type* type = cast<LogicTypeLLVM>(expr->returnType())->getType();
        if (!isa<PointerType>(type)) {
            throw type_exception("Argument to 'load' must be a pointer; got an expression of type '" + expr->returnType()->toString() + "'", this);
        }
        ptrType = cast<PointerType>(type);
        retType = new LogicTypeLLVM(ptrType->getElementType(), source);
    }
    LogicExpressionLoad::~LogicExpressionLoad() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionLoad::getExpr() {
        return expr;
    }
    
    string LogicExpressionLoad::toString() {
        return "*" + expr->toString();
    }
    
    LogicType* LogicExpressionLoad::returnType() {
        return retType;
    }
    
    void LogicExpressionLoad::checkTypes() {
        expr->checkTypes();
        
        // we have to type check in the constructor, so this is left blank.
    }
    
    void LogicExpressionLoad::toWhy3(ostream &out, Why3Data &data) {
        out << "(" << getWhy3TheoryName(ptrType) << ".load " << data.statepoint << " ";
        expr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionLoad::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
