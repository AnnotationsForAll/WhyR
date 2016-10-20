/*
 * expr_arg.cpp
 *
 *  Created on: Sep 19, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/exception.hpp>
#include <whyr/types.hpp>
#include <whyr/esc_why3.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const int classID = LOGIC_EXPR_VAR;
    LogicExpressionVariable::LogicExpressionVariable(Function* function, string name, NodeSource* source) : LogicExpression(source), function{function}, name{name} {
        id = classID;
        // Figure out exactly which function argument we're supposed to be, and store the result
        // first, check function arguments
        for (iplist<Argument>::iterator ii = function->getArgumentList().begin(); ii != function->getArgumentList().end(); ii++) {
            if (ii->getName().compare(StringRef(name)) == 0) {
                arg = &*ii;
                break;
            }
        }
        // next, check locals
        if (!arg) for (Function::iterator ii = function->begin(); ii != function->end(); ii++) {
            for (BasicBlock::iterator jj = ii->begin(); jj != ii->end(); jj++) {
                if (jj->getName().compare(StringRef(name)) == 0) {
                    arg = &*jj;
                    break;
                }
            }
        }
        // error if arg not found
        if (!arg) {
            throw syntax_exception("Unknown variable name '"+name+"' in function '"+function->getName().data()+"'", this);
        }
        // cache the return type
        retType = new LogicTypeLLVM(arg->getType(), source);
    }
    LogicExpressionVariable::~LogicExpressionVariable() {
        delete retType;
    }
    
    string LogicExpressionVariable::getName() {
        return name;
    }
    
    string LogicExpressionVariable::toString() {
        return "%" + name;
    }
    
    LogicType* LogicExpressionVariable::returnType() {
        return retType;
    }
    
    void LogicExpressionVariable::checkTypes() {}
    
    void LogicExpressionVariable::toWhy3(ostream &out, Why3Data &data) {
        if (data.calleeTheoryName) {
            out << getWhy3ArgName(data.calleeTheoryName, arg);
        } else {
            out << getWhy3VarName(arg);
        }
    }
    
    bool LogicExpressionVariable::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
