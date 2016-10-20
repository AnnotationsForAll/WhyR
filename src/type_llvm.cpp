/*
 * type_llvm.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_LLVM;
    LogicTypeLLVM::LogicTypeLLVM(Type* type, NodeSource* source) : LogicType(source), type{type} {
        id = classID;
    }
    LogicTypeLLVM::~LogicTypeLLVM() {}
    
    Type* LogicTypeLLVM::getType() {
        return type;
    }
    
    string LogicTypeLLVM::toString() {
        string s = "";
        raw_string_ostream sb(s);
        type->print(sb);
        return sb.str();
    }
    
    LogicType* LogicTypeLLVM::commonType(LogicType* other) {
        if (isa<LogicTypeLLVM>(other)) {
            if (this->equals(other)) { // If they're the exact same underlying type, they're valid
                return this;
            } else {
                LogicTypeLLVM* otherLLVM = cast<LogicTypeLLVM>(other);
                
                if (this->type->isPointerTy() && otherLLVM->type->isPointerTy()) { // void* to/from casts are always valid
                    if (this->type->getPointerElementType()->isVoidTy()) {
                        return this;
                    }
                    if (otherLLVM->type->getPointerElementType()->isVoidTy()) {
                        return other;
                    }
                }
                
                return NULL;
            }
        } else {
            return NULL;
        }
    }
    
    bool LogicTypeLLVM::equals(LogicType* other) {
        // types are equal only if they share the same underlying LLVM type too
        return isa<LogicTypeLLVM>(other) && this->type == cast<LogicTypeLLVM>(other)->type;
    }
    
    void LogicTypeLLVM::toWhy3(ostream &out, Why3Data &data) {
        getTypeInfo(*data.info, type);
        out << getWhy3FullName(type);
    }
    
    bool LogicTypeLLVM::classof(const LogicType* type) {
        return type->id == classID;
    }
}
