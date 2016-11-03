/*
 * type_bool.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_BOOL;
    LogicTypeBool::LogicTypeBool(NodeSource* source) : LogicType(source) {
        id = classID;
    }
    LogicTypeBool::~LogicTypeBool() {}
    
    string LogicTypeBool::toString() {
        return "bool";
    }
    
    bool LogicTypeBool::equals(LogicType* other) {
        return isa<LogicTypeBool>(other);
    }
    
    void LogicTypeBool::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("bool.Bool");
        out << "bool";
    }
    
    bool LogicTypeBool::classof(const LogicType* type) {
        return type->id == classID;
    }
}
