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

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_SET_INDEX;
    LogicExpressionSetIndex::LogicExpressionSetIndex(LogicExpression* lhs, LogicExpression* rhs, LogicExpression* value, NodeSource* source) : LogicExpression(source), lhs{lhs}, rhs{rhs}, value{value} {
        id = classID;
    }
    LogicExpressionSetIndex::~LogicExpressionSetIndex() {
        delete lhs;
        delete rhs;
        delete value;
    }
    
    LogicExpression* LogicExpressionSetIndex::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionSetIndex::getRight() {
        return rhs;
    }
    
    LogicExpression* LogicExpressionSetIndex::getValue() {
        return value;
    }
    
    string LogicExpressionSetIndex::toString() {
        return lhs->toString() + "[" + rhs->toString() + " = " + value->toString() + "]";
    }
    
    LogicType* LogicExpressionSetIndex::returnType() {
        return lhs->returnType();
    }
    
    void LogicExpressionSetIndex::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
        value->checkTypes();
        
        // lhs has to be indexable
        if (!isa<LogicTypeLLVM>(lhs->returnType()) || !(cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy() || cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy())) {
            throw type_exception("Operator 'set' expected aggregate type; got type '" + lhs->returnType()->toString() + "'", this);
        }
        
        // if we are an array type, rhs must be a logical int, and value must be the element's type
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy()) {
            if (!isa<LogicTypeInt>(rhs->returnType())) {
                throw type_exception("Operator 'set' expected index type of 'int'; got type '" + rhs->returnType()->toString() + "'", this);
            }
            
            Type* elemType = cast<LogicTypeLLVM>(lhs->returnType())->getType()->getArrayElementType();
            if (!isa<LogicTypeLLVM>(value->returnType()) || cast<LogicTypeLLVM>(value->returnType())->getType() != elemType) {
                throw type_exception("Operator 'set' expected value type compatible with '" + lhs->returnType()->toString() + "'; got type '" + value->returnType()->toString() + "'", this);
            }
        }
        
        // if we are an struct type, rhs must be a logical int CONSTANT, and value must be the element's type
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy()) {
            if (!isa<LogicExpressionIntegerConstant>(rhs)) {
                throw type_exception("Operator 'set' expected struct index constant of type 'int'; got '" + rhs->toString() + "'", this);
            }
            
            unsigned index = stoul(cast<LogicExpressionIntegerConstant>(rhs)->getValue());
            Type* elemType = cast<LogicTypeLLVM>(lhs->returnType())->getType()->getStructElementType(index);
            if (!isa<LogicTypeLLVM>(value->returnType()) || cast<LogicTypeLLVM>(value->returnType())->getType() != elemType) {
                throw type_exception("Operator 'set' expected value type compatible with '" + lhs->returnType()->toString() + "'; got type '" + value->returnType()->toString() + "'", this);
            }
        }
        
        // if we are a vector type, rhs must be a logical int, and value must be the element's type
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isVectorTy()) {
            if (!isa<LogicTypeInt>(rhs->returnType())) {
                throw type_exception("Operator 'set' expected index type of 'int'; got type '" + rhs->returnType()->toString() + "'", this);
            }
            
            Type* elemType = cast<LogicTypeLLVM>(lhs->returnType())->getType()->getVectorElementType();
            if (!isa<LogicTypeLLVM>(value->returnType()) || cast<LogicTypeLLVM>(value->returnType())->getType() != elemType) {
                throw type_exception("Operator 'set' expected value type compatible with '" + lhs->returnType()->toString() + "'; got type '" + value->returnType()->toString() + "'", this);
            }
        }
    }
    
    void LogicExpressionSetIndex::toWhy3(ostream &out, Why3Data &data) {
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy()) {
            lhs->toWhy3(out, data);
            out << "[";
            rhs->toWhy3(out, data);
            out << " <- ";
            value->toWhy3(out, data);
            out << "]";
        } else if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy()) {
            out << "{";
            lhs->toWhy3(out, data);
            unsigned index = stoul(cast<LogicExpressionIntegerConstant>(rhs)->getValue());
            out << " with " << getWhy3StructFieldName(data.module, cast<StructType>(cast<LogicTypeLLVM>(lhs->returnType())->getType()), index);
            out << " = ";
            value->toWhy3(out, data);
            out << ";}";
        } else if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isVectorTy()) {
            lhs->toWhy3(out, data);
            out << "[";
            rhs->toWhy3(out, data);
            out << " <- ";
            value->toWhy3(out, data);
            out << "]";
        }
    }
    
    bool LogicExpressionSetIndex::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
