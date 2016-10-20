/*
 * expr_bool.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/exception.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_BOOL;
    LogicExpressionBoolean::LogicExpressionBoolean(LogicExpression* value, NodeSource* source) : LogicExpression(source), value{value} {
        id = classID;
    }
    LogicExpressionBoolean::~LogicExpressionBoolean() {}
    
    LogicExpression* LogicExpressionBoolean::getValue() {
        return value;
    }
    
    string LogicExpressionBoolean::toString() {
        return value ? "\\true" : "\\false";
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionBoolean::returnType() {
        return &retType;
    }
    
    void LogicExpressionBoolean::checkTypes() {
        // This expression is only supposed to convert i1s into bools.
        if (!isa<LogicTypeLLVM>(value->returnType()) || !cast<LogicTypeLLVM>(value->returnType())->getType()->isIntegerTy(1)) {
            throw type_exception(("Argument to 'bool' must be of type 'i1'; got type '" + value->returnType()->toString() + "'"), this);
        }
    }
    
    void LogicExpressionBoolean::toWhy3(ostream &out, Why3Data &data) {
        out << "(";
        value->toWhy3(out, data);
        out << " = ";
        addLLVMIntConstant(out, data.module, Type::getInt1Ty(data.module->rawIR()->getContext()), "1");
        out << ")";
    }
    
    bool LogicExpressionBoolean::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
