/*
 * expr_llvm_vector_const.cpp
 *
 *  Created on: Oct 27, 2016
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
    
    static const int classID = LOGIC_EXPR_LLVM_VECTOR_CONST;
    LogicExpressionLLVMVectorConstant::LogicExpressionLLVMVectorConstant(Type* type, list<LogicExpression*>* elements, NodeSource* source) : LogicExpression(source), type{type}, elements{elements} {
        id = classID;
        
        retType = new LogicTypeLLVM(VectorType::get(type, elements->size()), source);
    }
    LogicExpressionLLVMVectorConstant::~LogicExpressionLLVMVectorConstant() {
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            delete *ii;
        }
        delete elements;
        delete retType;
    }
    
    Type* LogicExpressionLLVMVectorConstant::getType() {
        return type;
    }
    list<LogicExpression*>* LogicExpressionLLVMVectorConstant::getElems() {
        return elements;
    }
    
    string LogicExpressionLLVMVectorConstant::toString() {
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
    
    LogicType* LogicExpressionLLVMVectorConstant::returnType() {
        return retType;
    }
    
    void LogicExpressionLLVMVectorConstant::checkTypes() {
        // element type of vector must be vectorizable
        if (!VectorType::isValidElementType(type)) {
            throw type_exception("Type '" + LogicTypeLLVM(type).toString() + "' is not valid as a member of an LLVM vector", this);
        }
        
        // number of elements cannot be 0
        if (elements->empty()) {
            throw type_exception("LLVM vectors cannot be 0 elements long", this);
        }
        
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            (*ii)->checkTypes();
            
            // all element types must be the value type of the vector
            if (!isa<LogicTypeLLVM>((*ii)->returnType()) || cast<LogicTypeLLVM>((*ii)->returnType())->getType() != type) {
                throw type_exception("All elements of vector must be insertable into '"+retType->toString()+"'; got an element of type '"+(*ii)->returnType()->toString()+"'" , this);
            }
        }
    }
    
    void LogicExpressionLLVMVectorConstant::toWhy3(ostream &out, Why3Data &data) {
        out << getWhy3TheoryName(retType->getType()) << ".any_vector";
        int i = 0;
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            out << "[" << i << " <- ";
            (*ii)->toWhy3(out, data);
            out << "]";
            i++;
        }
    }
    
    bool LogicExpressionLLVMVectorConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
