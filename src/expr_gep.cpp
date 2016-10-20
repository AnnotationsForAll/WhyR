/*
 * expr_gep.cpp
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

#include <sstream>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_GEP;
    LogicExpressionGetElementPointer::LogicExpressionGetElementPointer(LogicExpression* expr, list<LogicExpression*>* elements, NodeSource* source) : LogicExpression(source), expr{expr}, elements{elements} {
        id = classID;
        
        // we type check up here, because we need to find retType
        
        // expr needs to be a pointer
        if (!isa<LogicTypeLLVM>(expr->returnType()) || !cast<LogicTypeLLVM>(expr->returnType())->getType()->isPointerTy()) {
            throw type_exception("Argument 1 of 'getelementptr' must be a pointer; got type '" + expr->returnType()->toString() + "'", this);
        }
        
        // find the return type
        Type* type = cast<LogicTypeLLVM>(expr->returnType())->getType();
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            
            if (type->isPointerTy()) {
                // an index into pointers can only occur first
                if (ii != elements->begin()) {
                    throw type_exception("Type '" + LogicTypeLLVM(type, source).toString() + "' cannot be indexed by 'getelementptr'; pointer types only valid as first index", this);
                }
                // type of index has to be integernal
                if (!isa<LogicTypeInt>((*ii)->returnType()) && (!isa<LogicTypeLLVM>((*ii)->returnType()) || !cast<LogicTypeLLVM>((*ii)->returnType())->getType()->isIntegerTy())) {
                    throw type_exception("Type '" + LogicTypeLLVM(type, source).toString() + "' cannot be indexed with type '" + (*ii)->returnType()->toString() + "'; must be an intergernal type", this);
                }
                type = type->getPointerElementType();
            } else if (type->isArrayTy()) {
                // type of index has to be integernal
                if (!isa<LogicTypeInt>((*ii)->returnType()) && (!isa<LogicTypeLLVM>((*ii)->returnType()) || !cast<LogicTypeLLVM>((*ii)->returnType())->getType()->isIntegerTy())) {
                    throw type_exception("Type '" + LogicTypeLLVM(type, source).toString() + "' cannot be indexed with type '" + (*ii)->returnType()->toString() + "'; must be an intergernal type", this);
                }
                type = type->getArrayElementType();
            } else {
                // type cannot be indexed
                throw type_exception("Type '" + LogicTypeLLVM(type, source).toString() + "' cannot be indexed by 'getelementptr'", this);
            }
        }
        if (elements->empty()) {
            type = type->getPointerElementType();
        }
        retType = new LogicTypeLLVM(PointerType::get(type, 0), source); // TODO: address spaces...
    }
    LogicExpressionGetElementPointer::~LogicExpressionGetElementPointer() {
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            delete *ii;
        }
        delete elements;
        delete retType;
    }
    
    LogicExpression* LogicExpressionGetElementPointer::getExpr() {
        return expr;
    }
    list<LogicExpression*>* LogicExpressionGetElementPointer::getElements() {
        return elements;
    }
    
    string LogicExpressionGetElementPointer::toString() {
        ostringstream out;
        out << "&" << expr->toString();
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            out << "[" << (*ii)->toString() << "]";
        }
        return out.str();
    }
    
    LogicType* LogicExpressionGetElementPointer::returnType() {
        return retType;
    }
    
    void LogicExpressionGetElementPointer::checkTypes() {
        
    }
    
    void LogicExpressionGetElementPointer::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("Pointer");
        
        out << "((cast (" << getWhy3TheoryName(cast<LogicTypeLLVM>(expr->returnType())->getType()) << ".offset_pointer ";
        expr->toWhy3(out, data);
        out << " (";
        Type* currentType = cast<LogicTypeLLVM>(expr->returnType())->getType();
        if (elements->empty()) {
            out << "0";
        }
        for (list<LogicExpression*>::iterator ii = elements->begin(); ii != elements->end(); ii++) {
            if (ii != elements->begin()) {
                out << " + ";
            }
            if (currentType->isPointerTy()) {
                out << "(" << getWhy3TheoryName(currentType) << ".size * ";
                if (isa<LogicTypeInt>((*ii)->returnType())) {
                    (*ii)->toWhy3(out, data);
                } else {
                    out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>((*ii)->returnType())->getType()) << ".to_int ";
                    (*ii)->toWhy3(out, data);
                    out << ")";
                }
                out << ")";
                currentType = currentType->getPointerElementType();
            } else if (currentType->isArrayTy()) {
                out << "(" << getWhy3TheoryName(currentType->getArrayElementType()) << ".size * ";
                if (isa<LogicTypeInt>((*ii)->returnType())) {
                    (*ii)->toWhy3(out, data);
                } else {
                    out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>((*ii)->returnType())->getType()) << ".to_int ";
                    (*ii)->toWhy3(out, data);
                    out << ")";
                }
                out << ")";
                currentType = currentType->getArrayElementType();
            }
        }
        out << "))):(" << getWhy3FullName(cast<LogicTypeLLVM>(retType)->getType()) << "))";
    }
    
    bool LogicExpressionGetElementPointer::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
