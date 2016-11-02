/*
 * logic.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/logic.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    // Most of these methods should be overwritten by subclasses.
    
    LogicType::LogicType(NodeSource* source) : source{source} {}
    LogicType::~LogicType() {}
    
    NodeSource* LogicType::getSource() {
        return source;
    }
    
    void LogicType::setSource(NodeSource* source) {
        this->source = source;
    }
    
    string LogicType::toString() {
        return "(unknown type)";
    }
    
    bool LogicType::classof(const LogicType* type) {
        return false;
    }
    
    bool LogicType::equals(LogicType* other) {
        return this == other;
    }
    
    void LogicType::toWhy3(ostream &out, Why3Data &data) {
        out << "(unknown type)";
    }
    
    LogicExpression::LogicExpression(NodeSource* source) : source{source} {}
    LogicExpression::~LogicExpression() {}
    
    NodeSource* LogicExpression::getSource() {
        return source;
    }
    
    void LogicExpression::setSource(NodeSource* source) {
        this->source = source;
    }
    
    string LogicExpression::toString() {
        return "(unknown expression)";
    }
    
    LogicType* LogicExpression::returnType() {
        return NULL;
    }
    
    bool LogicExpression::classof(const LogicExpression* expr) {
        return false;
    }
    
    void LogicExpression::checkTypes() {}
    
    void LogicExpression::toWhy3(ostream &out, Why3Data &data) {
        out << "(unknown why3)";
    }
    
    NodeSource::NodeSource(AnnotatedFunction* func, Instruction* inst, Metadata* metadata) : func{func}, inst{inst}, metadata{metadata} {}
    NodeSource::NodeSource(NodeSource* other) {
        *this = *other;
    }
}
