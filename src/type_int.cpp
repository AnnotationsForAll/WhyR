/*
 * type_int.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_INT;
    LogicTypeInt::LogicTypeInt(NodeSource* source) : LogicType(source) {
        id = classID;
    }
    LogicTypeInt::~LogicTypeInt() {}
    
    string LogicTypeInt::toString() {
        return "int";
    }
    
    LogicType* LogicTypeInt::commonType(LogicType* other) {
        return NULL;
    }
    
    bool LogicTypeInt::equals(LogicType* other) {
        return isa<LogicTypeInt>(other);
    }
    
    void LogicTypeInt::toWhy3(ostream &out, Why3Data &data) {
        out << "int";
    }
    
    bool LogicTypeInt::classof(const LogicType* type) {
        return type->id == classID;
    }
}
