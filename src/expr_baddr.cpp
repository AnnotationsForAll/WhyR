/*
 * expr_baddr.cpp
 *
 *  Created on: Oct 18, 2016
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
    
    static const int classID = LOGIC_EXPR_BADDR;
    LogicExpressionBlockAddress::LogicExpressionBlockAddress(Function* func, BasicBlock* block, NodeSource* source) : LogicExpression(source), func{func}, block{block} {
        id = classID;
        
        retType = new LogicTypeLLVM(PointerType::get(Type::getInt8Ty(source->func->rawIR()->getContext()), 0), source);
    }
    LogicExpressionBlockAddress::~LogicExpressionBlockAddress() {
        delete retType;
    }
    
    Function* LogicExpressionBlockAddress::getFunction() {
        return func;
    }
    
    BasicBlock* LogicExpressionBlockAddress::getBlock() {
        return block;
    }
    
    string LogicExpressionBlockAddress::toString() {
        return "blockaddress(@" + string(func->getName().data()) + ",%" + string(block->getName().data()) + ")";
    }
    
    LogicType* LogicExpressionBlockAddress::returnType() {
        return retType;
    }
    
    void LogicExpressionBlockAddress::checkTypes() {}
    
    void LogicExpressionBlockAddress::toWhy3(ostream &out, Why3Data &data) {
        if (func != data.source->func->rawIR()) data.info->funcsCalled.insert(func);
        data.info->usesBaddr = true;
        
        out << "(store_baddr " << getWhy3BlockName(data.module->getFunction(func), block) << ")";
    }
    
    bool LogicExpressionBlockAddress::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
