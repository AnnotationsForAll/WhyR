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
    
    static const int classID = LOGIC_EXPR_ARG;
    LogicExpressionArgument::LogicExpressionArgument(Function* function, string name, NodeSource* source) : LogicExpression(source), function{function}, name{name} {
        id = classID;
        // Figure out exactly which function argument we're supposed to be, and store the result
        for (iplist<Argument>::iterator ii = function->getArgumentList().begin(); ii != function->getArgumentList().end(); ii++) {
            if (ii->getName().compare(StringRef(name)) == 0) {
                arg = &*ii;
                break;
            }
        }
        if (!arg) {
            throw syntax_exception("Unknown argument name '"+name+"' in function '"+function->getName().data()+"'", this);
        }
        // cache the return type
        retType = new LogicTypeLLVM(arg->getType(), source);
    }
    LogicExpressionArgument::~LogicExpressionArgument() {
        delete retType;
    }
    
    string LogicExpressionArgument::getName() {
        return name;
    }
    
    string LogicExpressionArgument::toString() {
        return "%" + name;
    }
    
    LogicType* LogicExpressionArgument::returnType() {
        return retType;
    }
    
    void LogicExpressionArgument::checkTypes() {}
    
    void LogicExpressionArgument::toWhy3(ostream &out, Why3Data &data) {
        if (data.calleeTheoryName) {
            out << getWhy3ArgName(data.calleeTheoryName, arg);
        } else {
            out << getWhy3VarName(arg);
        }
    }
    
    bool LogicExpressionArgument::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
