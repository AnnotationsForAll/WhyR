/*
 * exception.cpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#include <whyr/exception.hpp>
#include <whyr/logic.hpp>
#include <whyr/module.hpp>

#include <cstring>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    whyr_exception::whyr_exception(string message, LogicExpression* source, NodeSource* node) : message{message}, source{source}, node{node} {
        if (source && !node) {
            this->node = source->getSource();
        }
    }
    whyr_exception::whyr_exception(const char* message, LogicExpression* source, NodeSource* node) : whyr_exception(string(message), source, node) {}
    
    whyr_exception::~whyr_exception() {}
    
    const char* whyr_exception::what() const throw () {
        return message.c_str();
    }
    
    LogicExpression* whyr_exception::getSource() {
        return source;
    }
    
    NodeSource* whyr_exception::getNodeSource() {
        return node;
    }
    
    void whyr_exception::printMessage(ostream &out) {
        if (node) {
            if (node->debugInfo.empty()) {
                out << "in module " << node->func->getModule()->rawIR()->getModuleIdentifier() << ": ";
                out << "in function " << node->func->rawIR()->getName().data() << ": ";
                if (node->inst) {
                    if (node->inst->getParent()->hasName()) {
                        out << "in block " << node->inst->getParent()->getName().data() << ": ";
                    }
                    out << "in instruction ";
                    if (node->inst->hasName()) {
                        out << "%" << node->inst->getName().data() << " = ";
                    }
                    out << node->inst->getOpcodeName() << ": ";
                }
            } else {
                LogicDebugInfo info = node->debugInfo.front();
                if (info.file) {
                    out << " In file '" << info.file << "': ";
                }
                if (info.line) {
                    out << "line " << info.line << ": ";
                }
                if (info.col1 && info.col2) {
                    out << "cols " << info.col1 << "-" << info.col2 << ":";
                } else if (info.col1) {
                    out << "col " << info.col1 << ":";
                }
            }
        }
        if (node || source) out << endl;
        if (source) {
            out << "    " << source->toString() << endl;
            out << "    ^" << endl;
        }
        out << "    " << message << endl;
    }
    
    syntax_exception::syntax_exception(const char* message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    syntax_exception::syntax_exception(string message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    syntax_exception::~syntax_exception() {}
    
    type_exception::type_exception(const char* message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    type_exception::type_exception(string message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    type_exception::~type_exception() {}
    
    llvm_exception::llvm_exception(const char* message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    llvm_exception::llvm_exception(string message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    llvm_exception::~llvm_exception() {}
    
    whyr_warning::whyr_warning(const char* message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    whyr_warning::whyr_warning(string message, LogicExpression* source, NodeSource* node) : whyr_exception(message, source, node) {}
    whyr_warning::~whyr_warning() {}
}
