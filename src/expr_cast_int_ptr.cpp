/*
 * expr_cast_int_ptr.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_INT_PTR;
    LogicExpressionIntToPointer::LogicExpressionIntToPointer(LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionIntToPointer::~LogicExpressionIntToPointer() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionIntToPointer::getExpr() {
        return expr;
    }
    
    string LogicExpressionIntToPointer::toString() {
        return "("+retType->toString()+") "+expr->toString();
    }
    
    LogicType* LogicExpressionIntToPointer::returnType() {
        return retType;
    }
    
    void LogicExpressionIntToPointer::checkTypes() {
        expr->checkTypes();
        
        // retType must be a pointer
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isPointerTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be a pointer", this);
        }
        
        // expr must be an LLVM int
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isIntegerTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be an LLVM int", this);
        }
    }
    
    void LogicExpressionIntToPointer::toWhy3(ostream &out, Why3Data &data) {
        unsigned ptrBits = data.module->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
        
        out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(retType)->getType()) << ".of_ptrint ";
        
        if (cast<LogicTypeLLVM>(expr->returnType())->getType()->getIntegerBitWidth() != ptrBits) {
            Type* ptrIntType = IntegerType::get(data.module->rawIR()->getContext(), ptrBits);
            
            out << "(" << getWhy3TheoryName(ptrIntType) << ".of_int (" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".to_uint ";
        }
        
        expr->toWhy3(out, data);
        
        if (cast<LogicTypeLLVM>(expr->returnType())->getType()->getIntegerBitWidth() != ptrBits) {
            out << "))";
        }
        
        out << ")";
    }
    
    bool LogicExpressionIntToPointer::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
