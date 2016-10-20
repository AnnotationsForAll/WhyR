/*
 * expr_bool_const.cpp
 *
 *  Created on: Sep 16, 2016
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
    
    static const int classID = LOGIC_EXPR_LLVM_ARRAY_CONST;
    LogicExpressionLLVMArrayConstant::LogicExpressionLLVMArrayConstant(Type* type, list<LogicExpression*>* elements, NodeSource* source) : LogicExpression(source), type{type}, elements{elements} {
        id = classID;
        
        retType = new LogicTypeLLVM(ArrayType::get(type, elements->size()), source);
    }
    LogicExpressionLLVMArrayConstant::~LogicExpressionLLVMArrayConstant() {
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            delete *ii;
        }
        delete elements;
        delete retType;
    }
    
    Type* LogicExpressionLLVMArrayConstant::getType() {
        return type;
    }
    list<LogicExpression*>* LogicExpressionLLVMArrayConstant::getElems() {
        return elements;
    }
    
    string LogicExpressionLLVMArrayConstant::toString() {
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
    
    LogicType* LogicExpressionLLVMArrayConstant::returnType() {
        return retType;
    }
    
    void LogicExpressionLLVMArrayConstant::checkTypes() {
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            (*ii)->checkTypes();
            
            // all element types must be the value type of the array
            if (!isa<LogicTypeLLVM>((*ii)->returnType()) || cast<LogicTypeLLVM>((*ii)->returnType())->getType() != type) {
                throw type_exception("All elements of array must be insertable into '"+retType->toString()+"; got an element of type '"+(*ii)->returnType()->toString()+"'" , this);
            }
        }
    }
    
    void LogicExpressionLLVMArrayConstant::toWhy3(ostream &out, Why3Data &data) {
        out << getWhy3TheoryName(retType->getType()) << ".any_array";
        int i = 0;
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            out << "[" << i << " <- ";
            (*ii)->toWhy3(out, data);
            out << "]";
            i++;
        }
    }
    
    bool LogicExpressionLLVMArrayConstant::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
