/*
 * expr_offset.cpp
 *
 *  Created on: Nov 2, 2016
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
    
    static const int classID = LOGIC_EXPR_OFFSET;
    LogicExpressionOffset::LogicExpressionOffset(LogicExpression* pointer, LogicExpression* offset, NodeSource* source) : LogicExpression(source), pointer{pointer}, offset{offset} {
        id = classID;
    }
    LogicExpressionOffset::~LogicExpressionOffset() {}
    
    LogicExpression* LogicExpressionOffset::getPointer() {
        return pointer;
    }
    
    LogicExpression* LogicExpressionOffset::getOffset() {
        return offset;
    }
    
    string LogicExpressionOffset::toString() {
        return pointer->toString() + " offset " + offset->toString();
    }
    
    LogicType* LogicExpressionOffset::returnType() {
        return pointer->returnType();
    }
    
    void LogicExpressionOffset::checkTypes() {
        pointer->checkTypes();
        offset->checkTypes();
        
        // pointer must be a llvm ptr or a set of ptrs
        if ((!isa<LogicTypeLLVM>(pointer->returnType()) || !cast<LogicTypeLLVM>(pointer->returnType())->getType()->isPointerTy()) && (!isa<LogicTypeSet>(pointer->returnType()) || !isa<LogicTypeLLVM>(cast<LogicTypeSet>(pointer->returnType())->getType()) || !cast<LogicTypeLLVM>(cast<LogicTypeSet>(pointer->returnType())->getType())->getType()->isPointerTy())) {
            throw type_exception("Argument 1 of 'offset' must be a pointer or set of pointers; got type '" + pointer->returnType()->toString() + "'", this);
        }
        
        // offset must be logical int or set of logical ints
        if (!isa<LogicTypeInt>(offset->returnType()) && (!isa<LogicTypeSet>(offset->returnType()) || !isa<LogicTypeInt>(cast<LogicTypeSet>(offset->returnType())->getType()))) {
            throw type_exception("Argument 2 of 'offset' must be an int or set of ints; got type '" + offset->returnType()->toString() + "'", this);
        }
        
        // if pointer it a set, offset must be, too. if offset is a set, pointer must be, too.
        if ((isa<LogicTypeSet>(pointer->returnType()) && !isa<LogicTypeSet>(offset->returnType())) || (!isa<LogicTypeSet>(pointer->returnType()) && isa<LogicTypeSet>(offset->returnType()))) {
            throw type_exception("Arguments to 'offset' must either both be sets or neither be sets; got types '" + pointer->returnType()->toString() + "' and '" + offset->returnType()->toString() + "'", this);
        }
    }
    
    void LogicExpressionOffset::toWhy3(ostream &out, Why3Data &data) {
        if (isa<LogicTypeSet>(pointer->returnType())) {
            data.importsNeeded.insert("MemorySet");
            out << "(offset_memset ";
        } else {
            data.importsNeeded.insert("Pointer");
            out << "(offset_pointer ";
        }
        
        pointer->toWhy3(out, data);
        out << " ";
        offset->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionOffset::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
