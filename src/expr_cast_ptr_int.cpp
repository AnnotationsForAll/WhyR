/*
 * expr_cast_ptr_int.cpp
 *
 *  Created on: Oct 13, 2016
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
    
    static const int classID = LOGIC_EXPR_CAST_PTR_INT;
    LogicExpressionPointerToInt::LogicExpressionPointerToInt(LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionPointerToInt::~LogicExpressionPointerToInt() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionPointerToInt::getExpr() {
        return expr;
    }
    
    string LogicExpressionPointerToInt::toString() {
        return "("+retType->toString()+") "+expr->toString();
    }
    
    LogicType* LogicExpressionPointerToInt::returnType() {
        return retType;
    }
    
    void LogicExpressionPointerToInt::checkTypes() {
        expr->checkTypes();
        
        // retType must be an LLVM int
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be an LLVM int", this);
        }
        
        // expr must be a pointer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isPointerTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be a pointer", this);
        }
    }
    
    void LogicExpressionPointerToInt::toWhy3(ostream &out, Why3Data &data) {
        unsigned ptrBits = data.module->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
        
        if (cast<LogicTypeLLVM>(retType)->getType()->getIntegerBitWidth() != ptrBits) {
            Type* ptrIntType = IntegerType::get(data.module->rawIR()->getContext(), ptrBits);
            
            out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(retType)->getType()) << ".of_int (" << getWhy3TheoryName(ptrIntType) << ".to_uint ";
        }
        
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_ptrint ";
        expr->toWhy3(out, data);
        out << ")";
        
        if (cast<LogicTypeLLVM>(expr->returnType())->getType()->getIntegerBitWidth() != ptrBits) {
            out << "))";
        }
    }
    
    bool LogicExpressionPointerToInt::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
