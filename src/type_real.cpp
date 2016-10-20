/*
 * type_real.cpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>
#include <whyr/types.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_TYPE_REAL;
    LogicTypeReal::LogicTypeReal(NodeSource* source) : LogicType(source) {
        id = classID;
    }
    LogicTypeReal::~LogicTypeReal() {}
    
    string LogicTypeReal::toString() {
        return "real";
    }
    
    LogicType* LogicTypeReal::commonType(LogicType* other) {
        return NULL;
    }
    
    bool LogicTypeReal::equals(LogicType* other) {
        return isa<LogicTypeReal>(other);
    }
    
    void LogicTypeReal::toWhy3(ostream &out, Why3Data &data) {
        out << "real";
    }
    
    bool LogicTypeReal::classof(const LogicType* type) {
        return type->id == classID;
    }
}
