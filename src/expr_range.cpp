/*
 * expr_range.cpp
 *
 *  Created on: Nov 2, 2016
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
    
    static const int classID = LOGIC_EXPR_RANGE;
    LogicExpressionRange::LogicExpressionRange(LogicExpression* begin, LogicExpression* end, NodeSource* source) : LogicExpression(source), begin{begin}, end{end} {
        id = classID;
        
        retType = new LogicTypeSet(new LogicTypeInt(source),source);
    }
    LogicExpressionRange::~LogicExpressionRange() {}
    
    LogicExpression* LogicExpressionRange::getBegin() {
        return begin;
    }
    
    LogicExpression* LogicExpressionRange::getEnd() {
        return end;
    }
    
    string LogicExpressionRange::toString() {
        return begin->toString() + ".." + end->toString();
    }
    
    LogicType* LogicExpressionRange::returnType() {
        return retType;
    }
    
    void LogicExpressionRange::checkTypes() {
        begin->checkTypes();
        end->checkTypes();
        
        // begin must be a logic int
        if (!isa<LogicTypeInt>(begin->returnType())) {
            throw type_exception("argument 1 of 'range' expected to be of type 'int'; got type '" + begin->returnType()->toString() + "'", this);
        }
        
        // end must be a logic int
        if (!isa<LogicTypeInt>(end->returnType())) {
            throw type_exception("argument 2 of 'range' expected to be of type 'int'; got type '" + end->returnType()->toString() + "'", this);
        }
    }
    
    void LogicExpressionRange::toWhy3(ostream &out, Why3Data &data) {
        data.importsNeeded.insert("Range");
        
        out << "(range ";
        begin->toWhy3(out, data);
        out << " ";
        end->toWhy3(out, data);
        out << ")";
    }
    
    bool LogicExpressionRange::classof(const LogicExpression* expr) {
        return expr->id == classID;
    }
}
