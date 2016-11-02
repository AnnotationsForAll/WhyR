/*
 * expr_subset.cpp
 *
 *  Created on: Nov 1, 2016
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
    
    static const int classID = LOGIC_EXPR_SUBSET;
    LogicExpressionSubset::LogicExpressionSubset(LogicExpression* subExpr, LogicExpression* superExpr, NodeSource* source) : LogicExpression(source), subExpr{subExpr}, superExpr{superExpr} {
        id = classID;
    }
    LogicExpressionSubset::~LogicExpressionSubset() {}
    
    LogicExpression* LogicExpressionSubset::getSubExpr() {
        return subExpr;
    }
    
    LogicExpression* LogicExpressionSubset::getSuperExpr() {
        return superExpr;
    }
    
    string LogicExpressionSubset::toString() {
        return subExpr->toString() + " in " + superExpr->toString();
    }
    
    static LogicTypeBool retType;
    LogicType* LogicExpressionSubset::returnType() {
        return &retType;
    }
    
    void LogicExpressionSubset::checkTypes() {
        subExpr->checkTypes();
        superExpr->checkTypes();
        
        // subExpr must be a set
        if (!isa<LogicTypeSet>(subExpr->returnType())) {
            throw type_exception("Subset argument of 'subset' expected to be of type 'set'; got type '" + subExpr->returnType()->toString() + "'", this);
        }
        
        // superExpr must be a set
        if (!isa<LogicTypeSet>(superExpr->returnType())) {
            throw type_exception("Superset argument of 'subset' expected to be of type 'set'; got type '" + superExpr->returnType()->toString() + "'", this);
        }
        
        // arguments must be the same type
        if (!subExpr->returnType()->equals(superExpr->returnType())) {
            throw type_exception("Arguments to 'subset' must be of the same type; got types '" + subExpr->returnType()->toString() + "' and '" + superExpr->returnType()->toString() + "'", this);
        }
    }
    
    void LogicExpressionSubset::toWhy3(ostream &out, Why3Data &data) {
        string theory;
        if (isa<LogicTypeLLVM>(cast<LogicTypeSet>(subExpr->returnType())->getType()) && cast<LogicTypeLLVM>(cast<LogicTypeSet>(subExpr->returnType())->getType())->getType()->isPointerTy()) {
            data.importsNeeded.insert("MemorySet");
            theory = "MemorySet";
        } else {
            data.importsNeeded.insert("set.Set");
            theory = "Set";
        }
        
        out << "(" << theory << ".subset ";
        subExpr->toWhy3(out, data);
        out << " ";
        superExpr->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionSubset::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
