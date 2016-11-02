/*
 * type_set.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_SET;
    LogicTypeSet::LogicTypeSet(LogicType* type, NodeSource* source) : LogicType(source), type{type} {
        id = classID;
    }
    LogicTypeSet::~LogicTypeSet() {}
    
    string LogicTypeSet::toString() {
        string s = "";
        raw_string_ostream sb(s);
        sb << "set<";
        sb << type->toString();
        sb << ">";
        return sb.str();
    }
    
    bool LogicTypeSet::equals(LogicType* other) {
        // Two of these have an equal type if they have the same member type
        return isa<LogicTypeSet>(other) && this->type->equals(cast<LogicTypeSet>(other)->type);
    }
    
    void LogicTypeSet::toWhy3(ostream &out, Why3Data &data) {
        if (isa<LogicTypeLLVM>(type) && cast<LogicTypeLLVM>(type)->getType()->isPointerTy()) {
            out << "(mem_set ";
            LogicTypeLLVM(cast<LogicTypeLLVM>(type)->getType()->getPointerElementType()).toWhy3(out, data);
        } else {
            out << "(set ";
            type->toWhy3(out, data);
        }
        out << ")";
    }
    
    bool LogicTypeSet::classof(const LogicType* type) {
        return type->id == classID;
    }
    
    LogicType* LogicTypeSet::getType() {
        return type;
    }
}
