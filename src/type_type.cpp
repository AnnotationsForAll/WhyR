/*
 * type_type.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_TYPE;
    LogicTypeType::LogicTypeType(LogicType* type, NodeSource* source) : LogicType(source), type{type} {
        id = classID;
    }
    LogicTypeType::~LogicTypeType() {}
    
    string LogicTypeType::toString() {
        string s = "";
        raw_string_ostream sb(s);
        sb << "type<";
        sb << type->toString();
        sb << ">";
        return sb.str();
    }
    
    bool LogicTypeType::equals(LogicType* other) {
        // Two of these have an equal type if they have the same member type
        return isa<LogicTypeType>(other) && this->type->equals(cast<LogicTypeType>(other)->type);
    }
    
    bool LogicTypeType::classof(const LogicType* type) {
        return type->id == classID;
    }
    
    LogicType* LogicTypeType::getType() {
        return type;
    }
}
