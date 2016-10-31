/*
 * expr_set.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_SET;
    LogicExpressionCreateSet::LogicExpressionCreateSet(LogicType* baseType, list<LogicExpression*> &elems, NodeSource* source) : LogicExpression(source), setType{LogicTypeSet(baseType)}, baseType{baseType}, elems{elems} {
        id = classID;
        
        
    }
    
    LogicExpressionCreateSet::~LogicExpressionCreateSet() {
        for (list<LogicExpression*>::iterator ii = elems.begin(); ii != elems.end(); ii++) {
            delete *ii;
        }
        delete baseType;
    }
    
    list<LogicExpression*>* LogicExpressionCreateSet::getElements() {
        return &elems;
    }
    
    LogicType* LogicExpressionCreateSet::getBaseType() {
        return baseType;
    }
    
    LogicType* LogicExpressionCreateSet::returnType() {
        return &setType;
    }
    
    string LogicExpressionCreateSet::toString() {
        string s = "";
        raw_string_ostream sb(s);
        sb << "((" << setType.toString() << ") {";
        bool first = true;
        for (list<LogicExpression*>::iterator ii = elems.begin(); ii != elems.end(); ii++) {
            if (first) {
                first = false;
            } else {
                sb << ", ";
            }
            
            LogicExpression* expr = *ii;
            sb << expr->toString();
        }
        sb << "})";
        return sb.str();
    }
    
    void LogicExpressionCreateSet::checkTypes() {
        // All types of elements in this set should agree with the base type of this set.
        for (list<LogicExpression*>::iterator ii = elems.begin(); ii != elems.end(); ii++) {
            (*ii)->checkTypes();
            LogicType* type = LogicType::commonType(baseType, (*ii)->returnType());
            if (!type || !type->equals(baseType)) {
                throw type_exception(("Cannot create a set of type '" + baseType->toString() + "' with an element of type '" + (*ii)->returnType()->toString() + "'"), this);
            }
        }
    }
    
    void LogicExpressionCreateSet::toWhy3(ostream &out, Why3Data &data) {
        string theory;
        if (isa<LogicTypeLLVM>(baseType) && cast<LogicTypeLLVM>(baseType)->getType()->isPointerTy()) {
            data.importsNeeded.insert("MemorySet");
            theory = "MemorySet";
        } else {
            data.importsNeeded.insert("set.Set");
            theory = "Set";
        }
        
        for (list<LogicExpression*>::iterator ii = elems.begin(); ii != elems.end(); ii++) {
            out << "(" << theory << ".add ";
            (*ii)->toWhy3(out, data);
            out << " ";
        }
        
        out << "(" << theory << ".empty:";
        setType.toWhy3(out, data);
        out << ")";
        
        for (list<LogicExpression*>::iterator ii = elems.begin(); ii != elems.end(); ii++) {
            out << ")";
        }
    }
    
    bool LogicExpressionCreateSet::classof(const LogicExpression* type) {
        return type->id == classID;
    }
}
