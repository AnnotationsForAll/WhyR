/*
 * expr_cast_ptr_ptr.cpp
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
    
    static const int classID = LOGIC_EXPR_CAST_PTR_PTR;
    LogicExpressionPointerToPointer::LogicExpressionPointerToPointer(LogicExpression* expr, LogicType* retType, NodeSource* source) : LogicExpression(source), expr{expr}, retType{retType} {
        id = classID;
    }
    LogicExpressionPointerToPointer::~LogicExpressionPointerToPointer() {
        delete expr;
    }
    
    LogicExpression* LogicExpressionPointerToPointer::getExpr() {
        return expr;
    }
    
    string LogicExpressionPointerToPointer::toString() {
        return "("+retType->toString()+") "+expr->toString();
    }
    
    LogicType* LogicExpressionPointerToPointer::returnType() {
        return retType;
    }
    
    void LogicExpressionPointerToPointer::checkTypes() {
        expr->checkTypes();
        
        // retType must be a pointer
        if (!isa<LogicTypeLLVM>(retType) || !cast<LogicTypeLLVM>(retType)->getType()->isPointerTy()) {
            throw type_exception("Cannot cast to type '"+retType->toString()+"'; it must be a pointer", this);
        }
        
        // expr must be a pointer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isPointerTy()) {
            throw type_exception("Cannot cast from type '"+expr->returnType()->toString()+"'; it must be a pointer", this);
        }
    }
    
    void LogicExpressionPointerToPointer::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("Pointer");
        
        out << "((cast ";
        expr->toWhy3(out, data);
        out << "):(" << getWhy3FullName(cast<LogicTypeLLVM>(retType)->getType()) << "))";
    }
    
    bool LogicExpressionPointerToPointer::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
