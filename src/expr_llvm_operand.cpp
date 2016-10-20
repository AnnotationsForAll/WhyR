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
    
    static const int classID = LOGIC_EXPR_OPERAND;
    LogicExpressionLLVMOperand::LogicExpressionLLVMOperand(Value* operand, NodeSource* source) : LogicExpression(source), operand{operand} {
        id = classID;
        
        retType = new LogicTypeLLVM(operand->getType());
    }
    LogicExpressionLLVMOperand::~LogicExpressionLLVMOperand() {}

    Value* LogicExpressionLLVMOperand::getOperand() {
        return operand;
    }
    
    string LogicExpressionLLVMOperand::toString() {
        string s = "";
        raw_string_ostream sb(s);
        operand->print(sb);
        return sb.str();
    }
    
    LogicType* LogicExpressionLLVMOperand::returnType() {
        return retType;
    }
    
    void LogicExpressionLLVMOperand::checkTypes() {}
    
    void LogicExpressionLLVMOperand::toWhy3(ostream &out, Why3Data &data) {
        // because this is just a wrapper around a Value*, we can use addOperand directly to generate Why3.
        getTypeInfo(*data.info, operand->getType());
        addOperand(out, data.module, operand);
    }
    
    bool LogicExpressionLLVMOperand::classof(const LogicExpression* type) {
        return type->id == classID;
    }
}
