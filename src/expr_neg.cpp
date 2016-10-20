/*
 * expr_neg.cpp
 *
 *  Created on: Oct 3, 2016
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
    
    static const int classID = LOGIC_EXPR_NEG;
    LogicExpressionNegate::LogicExpressionNegate(LogicExpression* rhs, NodeSource* source) : LogicExpression(source), rhs{rhs} {
        id = classID;
    }
    LogicExpressionNegate::~LogicExpressionNegate() {
        delete rhs;
    }
    
    LogicExpression* LogicExpressionNegate::getValue() {
        return rhs;
    }
    
    string LogicExpressionNegate::toString() {
        return "-" + rhs->toString();
    }
    
    LogicType* LogicExpressionNegate::returnType() {
        return rhs->returnType();
    }
    
    void LogicExpressionNegate::checkTypes() {
        rhs->checkTypes();
        
        // The operands need to be numeric in some manner
        switch (returnType()->id) {
            case LOGIC_TYPE_LLVM: {
                // fail only if LLVM type is non-numeric
                Type* ty = cast<LogicTypeLLVM>(returnType())->getType();
                if (!(ty->isIntegerTy() || ty->isFloatingPointTy())) {
                    goto non_numeric_type;
                }
                break;
            }
            case LOGIC_TYPE_INT:
            case LOGIC_TYPE_REAL: {
                // always valid
                break;
            }
            default: {
                // not a numeric type; throw an error
                non_numeric_type:
                throw type_exception(("Expression '" + toString() + "' undefined for non-numeric type '" + returnType()->toString() + "'"), this);
            }
        }
    }
    
    void LogicExpressionNegate::toWhy3(ostream &out, Why3Data &data) {
        if (isa<LogicTypeLLVM>(rhs->returnType()) && cast<LogicTypeLLVM>(rhs->returnType())->getType()->isIntegerTy()) {
            out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(rhs->returnType())->getType()) << ".sub ";
            addLLVMIntConstant(out, data.module, cast<LogicTypeLLVM>(rhs->returnType())->getType(), "0");
            out << " ";
            rhs->toWhy3(out, data);
            out << ")";
        } else if (isa<LogicTypeLLVM>(rhs->returnType()) && cast<LogicTypeLLVM>(rhs->returnType())->getType()->isFloatingPointTy()) {
            data.importsNeeded.insert("floating_point.Rounding");
            out << "(" << getWhy3TheoryName(cast<LogicTypeLLVM>(rhs->returnType())->getType()) << ".fsub Rounding.NearestTiesToEven ";
            addLLVMFloatConstant(out, data.module, cast<LogicTypeLLVM>(rhs->returnType())->getType(), "0.0");
            out << " ";
            rhs->toWhy3(out, data);
            out << ")";
        } else if (isa<LogicTypeInt>(rhs->returnType())) {
            out << "(- ";
            rhs->toWhy3(out, data);
            out << ")";
        } else if (isa<LogicTypeReal>(rhs->returnType())) {
            out << "(-. ";
            rhs->toWhy3(out, data);
            out << ")";
        } else {
            // error?
           throw whyr_exception("internal error: unknown operand type to LogicExpressionNegate", this);
        }
    }
    
    bool LogicExpressionNegate::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
