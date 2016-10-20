/*
 * expr_llvm_const.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_LLVM;
    LogicExpressionLLVMConstant::LogicExpressionLLVMConstant(Constant* value, NodeSource* source) : LogicExpression(source), value{value} {
        id = classID;
    }
    LogicExpressionLLVMConstant::~LogicExpressionLLVMConstant() {}

    Constant* LogicExpressionLLVMConstant::getValue() {
        return value;
    }
    
    string LogicExpressionLLVMConstant::toString() {
        string s = "";
        raw_string_ostream sb(s);
        value->print(sb);
        return sb.str();
    }
    
    LogicType* LogicExpressionLLVMConstant::returnType() {
        return new LogicTypeLLVM(value->getType());
    }
    
    void LogicExpressionLLVMConstant::checkTypes() {}
    
    void LogicExpressionLLVMConstant::toWhy3(ostream &out, Why3Data &data) {
        // because this is just a wrapper around a Constant*, we can use addConstant directly to generate Why3.
        getTypeInfo(*data.info, value->getType());
        addOperand(out, data.module, value);
    }
    
    bool LogicExpressionLLVMConstant::classof(const LogicExpression* type) {
        return type->id == classID;
    }
}
