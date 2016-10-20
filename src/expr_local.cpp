/*
 * expr_local.cpp
 *
 *  Created on: Sep 30, 2016
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
    
    static const int classID = LOGIC_EXPR_LOCAL;
    LogicExpressionLocal::LogicExpressionLocal(string name, NodeSource* source) : LogicExpression(source), name{name} {
        id = classID;
        unordered_map<string, list<LogicLocal*>>::iterator ii = source->logicLocals.find(name);
        if (ii == source->logicLocals.end() || ii->second.empty()) {
            throw syntax_exception("Unknown local variable '"+name+"' in expression", this);
        } else {
            local = ii->second.back();
        }
    }
    LogicExpressionLocal::~LogicExpressionLocal() {}
    
    string LogicExpressionLocal::getName() {
        return name;
    }
    
    string LogicExpressionLocal::toString() {
        return "$" + name;
    }
    
    LogicType* LogicExpressionLocal::returnType() {
        return local->type;
    }
    
    void LogicExpressionLocal::checkTypes() {}
    
    void LogicExpressionLocal::toWhy3(ostream &out, Why3Data &data) {
        out << getWhy3LocalName(local);
    }
    
    bool LogicExpressionLocal::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
