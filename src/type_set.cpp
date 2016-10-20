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
    
    LogicType* LogicTypeSet::commonType(LogicType* other) {
        // this has a common type only if the other type is a set and the element classes have a common type
        if (isa<LogicTypeSet>(other)) {
            LogicType* common = LogicType::commonType(this->type, cast<LogicTypeSet>(other)->type);
            if (common) {
                return new LogicTypeSet(common, source);
            }
        }
        return NULL;
    }
    
    bool LogicTypeSet::equals(LogicType* other) {
        // Two of these have an equal type if they have the same member type
        return isa<LogicTypeSet>(other) && this->type->equals(cast<LogicTypeSet>(other)->type);
    }
    
    bool LogicTypeSet::classof(const LogicType* type) {
        return type->id == classID;
    }
    
    LogicType* LogicTypeSet::getType() {
        return type;
    }
}
