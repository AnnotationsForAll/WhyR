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
    
    static const int classID = LOGIC_EXPR_GET_INDEX;
    LogicExpressionGetIndex::LogicExpressionGetIndex(LogicExpression* lhs, LogicExpression* rhs, NodeSource* source) : LogicExpression(source), lhs{lhs}, rhs{rhs} {
        id = classID;
        
        // we have to type check up here because the retType depends on it
        
        // lhs has to be indexable
        if (!isa<LogicTypeLLVM>(lhs->returnType()) || !(cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy() || cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy() || cast<LogicTypeLLVM>(lhs->returnType())->getType()->isPointerTy())) {
            throw type_exception("Operator 'get' expected indexable type; got type '" + lhs->returnType()->toString() + "'", this);
        }
        
        // if we are an array type, rhs must be a logical int
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy()) {
            if (!isa<LogicTypeInt>(rhs->returnType())) {
                throw type_exception("Operator 'get' expected index type of 'int'; got type '" + rhs->returnType()->toString() + "'", this);
            } else {
                retType = new LogicTypeLLVM(cast<LogicTypeLLVM>(lhs->returnType())->getType()->getArrayElementType(), source);
            }
        }
        
        // if we are an struct type, rhs must be a logical int CONSTANT
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy()) {
            if (!isa<LogicExpressionIntegerConstant>(rhs)) {
                throw type_exception("Operator 'get' expected struct index constant of type 'int'; got '" + rhs->toString() + "'", this);
            } else {
                unsigned index = stoul(cast<LogicExpressionIntegerConstant>(rhs)->getValue());
                retType = new LogicTypeLLVM(cast<LogicTypeLLVM>(lhs->returnType())->getType()->getStructElementType(index), source);
            }
        }
        
        // if we are a pointer type, rhs must be a logical int
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isPointerTy()) {
            if (!isa<LogicTypeInt>(rhs->returnType())) {
                throw type_exception("Operator 'get' expected index type of 'int'; got type '" + rhs->returnType()->toString() + "'", this);
            } else {
                retType = new LogicTypeLLVM(cast<LogicTypeLLVM>(lhs->returnType())->getType()->getPointerElementType(), source);
            }
        }
        
        // if we are a vector type, rhs must be a logical int
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isVectorTy()) {
            if (!isa<LogicTypeInt>(rhs->returnType())) {
                throw type_exception("Operator 'get' expected index type of 'int'; got type '" + rhs->returnType()->toString() + "'", this);
            } else {
                retType = new LogicTypeLLVM(cast<LogicTypeLLVM>(lhs->returnType())->getType()->getVectorElementType(), source);
            }
        }
    }
    LogicExpressionGetIndex::~LogicExpressionGetIndex() {
        delete lhs;
        delete rhs;
        delete retType;
    }
    
    LogicExpression* LogicExpressionGetIndex::getLeft() {
        return lhs;
    }
    
    LogicExpression* LogicExpressionGetIndex::getRight() {
        return rhs;
    }
    
    string LogicExpressionGetIndex::toString() {
        return lhs->toString() + "[" + rhs->toString() + "]";
    }
    
    LogicType* LogicExpressionGetIndex::returnType() {
        return retType;
    }
    
    void LogicExpressionGetIndex::checkTypes() {
        lhs->checkTypes();
        rhs->checkTypes();
    }
    
    void LogicExpressionGetIndex::toWhy3(ostream &out, Why3Data &data) {
        if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isArrayTy()) {
            lhs->toWhy3(out, data);
            out << "[";
            rhs->toWhy3(out, data);
            out << "]";
        }  else if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isStructTy()) {
            lhs->toWhy3(out, data);
            unsigned index = stoul(cast<LogicExpressionIntegerConstant>(rhs)->getValue());
            out << "." << getWhy3StructFieldName(data.module, cast<StructType>(cast<LogicTypeLLVM>(lhs->returnType())->getType()), index);
        } else if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isPointerTy()) {
            out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(lhs->returnType())->getType()) << ".load " << data.statepoint << " ";
            out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(lhs->returnType())->getType()) << ".offset_pointer ";
            lhs->toWhy3(out, data);
            out << " (" << getWhy3TheoryName(cast<LogicTypeLLVM>(lhs->returnType())->getType()) << ".elem_size * ";
            rhs->toWhy3(out, data);
            out << ")))";
        } else if (isa<LogicTypeLLVM>(lhs->returnType()) && cast<LogicTypeLLVM>(lhs->returnType())->getType()->isVectorTy()) {
            lhs->toWhy3(out, data);
            out << "[";
            rhs->toWhy3(out, data);
            out << "]";
        }
    }
    
    bool LogicExpressionGetIndex::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
