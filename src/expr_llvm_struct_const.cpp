/*
 * expr_llvm_struct_const.cpp
 *
 *  Created on: Oct 24, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>

#include <sstream>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_LLVM_STRUCT_CONST;
    LogicExpressionLLVMStructConstant::LogicExpressionLLVMStructConstant(LogicType* retType, list<LogicExpression*>* elements, NodeSource* source) : LogicExpression(source), retType{retType}, elements{elements} {
        id = classID;
    }
    LogicExpressionLLVMStructConstant::~LogicExpressionLLVMStructConstant() {
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            delete *ii;
        }
        delete elements;
    }
    
    list<LogicExpression*>* LogicExpressionLLVMStructConstant::getElems() {
        return elements;
    }
    
    string LogicExpressionLLVMStructConstant::toString() {
        ostringstream out;
        out << "{";
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            if (ii != elements->begin()) {
                out << ",";
            }
            out << (*ii)->toString();
        }
        out << "}";
        return out.str();
    }
    
    LogicType* LogicExpressionLLVMStructConstant::returnType() {
        return retType;
    }
    
    void LogicExpressionLLVMStructConstant::checkTypes() {
        // the return type must be an LLVM struct
        if (!isa<LogicTypeLLVM>(retType) || !isa<StructType>(cast<LogicTypeLLVM>(retType)->getType())) {
            throw type_exception("Return type of function 'struct' must be an LLVM struct type; got type '" + retType->toString() + "'" , this);
        }
        StructType* type = cast<StructType>(cast<LogicTypeLLVM>(retType)->getType());
        
        // the length of the struct and the items given must match
        if (type->getStructNumElements() != elements->size()) {
            throw type_exception("Struct type '" + retType->toString() + "' has " + to_string(type->getStructNumElements()) + " elements; got " + to_string(elements->size()) , this);
        }
        
        // each element must correspond to the correct type
        int i = 0;
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            (*ii)->checkTypes();
            
            // all element types must be the value type of the array
            if (!isa<LogicTypeLLVM>((*ii)->returnType()) || cast<LogicTypeLLVM>((*ii)->returnType())->getType() != type->getStructElementType(i)) {
                throw type_exception("Element " + to_string(i) + " of struct type '" + retType->toString() + "' is of type '"+LogicTypeLLVM(type->getStructElementType(i)).toString()+"; got type '"+(*ii)->returnType()->toString()+"'" , this);
            }
            
            i++;
        }
    }
    
    void LogicExpressionLLVMStructConstant::toWhy3(ostream &out, Why3Data &data) {
        StructType* type = cast<StructType>(cast<LogicTypeLLVM>(retType)->getType());
        
        out << "({ ";
        int i = 0;
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            out << getWhy3StructFieldName(data.module, type, i) << " = ";
            (*ii)->toWhy3(out, data);
            out << "; ";
            i++;
        }
        out << "}:" << getWhy3FullName(type) << ")";
    }
    
    bool LogicExpressionLLVMStructConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
