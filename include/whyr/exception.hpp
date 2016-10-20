/*
 * exception.hpp
 *
 *  Created on: Sep 12, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_EXCEPTION_HPP_
#define INCLUDE_WHYR_EXCEPTION_HPP_

/**
 * This file lists all the exceptions WhyR can throw.
 */

#include "whyr.hpp"

#include <exception>
#include <string>
#include <ostream>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    class LogicExpression; class NodeSource;
    
    /**
     * This is the base class for WhyR exceptions.
     * This is an acceptable class to throw for internal errors; but throw subclasses for user errors.
     */
    class whyr_exception : public exception {
    protected:
        string message;
        LogicExpression* source;
        NodeSource* node;
    public:
        whyr_exception(const char* message, LogicExpression* source = NULL, NodeSource* node = NULL);
        whyr_exception(string message, LogicExpression* source = NULL, NodeSource* node = NULL);
        virtual ~whyr_exception();
        
        virtual const char* what() const throw ();
        virtual LogicExpression* getSource();
        virtual NodeSource* getNodeSource();
        virtual void printMessage(ostream &out);
    };
    
    /**
     * This exception is a syntax error, usually due to incorrect setup of WhyR metadata nodes.
     */
    class syntax_exception : public whyr_exception {
    public:
        syntax_exception(const char* message, LogicExpression* source = NULL, NodeSource* node = NULL);
        syntax_exception(string message, LogicExpression* source = NULL, NodeSource* node = NULL);
        virtual ~syntax_exception();
    };
    
    /**
     * This exception is a type-safety violation, thrown during type checking. 
     */
    class type_exception : public whyr_exception {
    public:
        type_exception(const char* message, LogicExpression* source = NULL, NodeSource* node = NULL);
        type_exception(string message, LogicExpression* source = NULL, NodeSource* node = NULL);
        virtual ~type_exception();
    };
    
    /**
     * This exception is caused by unparsable LLVM when converting it to a model.
     * This is usually caused by unknown types, opcodes, etc.
     */
    class llvm_exception : public whyr_exception {
    public:
        llvm_exception(const char* message, LogicExpression* source = NULL, NodeSource* node = NULL);
        llvm_exception(string message, LogicExpression* source = NULL, NodeSource* node = NULL);
        virtual ~llvm_exception();
    };
    
    /**
     * This exception is a warning, never thrown.
     * To add a warning, create a whyr_warning and add it to the WhyRSettings's warnings list.
     * Try to not throw whyr_warnings, unless you're dealing with WhyRSettings and werror!
     */
    class whyr_warning : public whyr_exception {
    public:
        whyr_warning(const char* message, LogicExpression* source = NULL, NodeSource* node = NULL);
        whyr_warning(string message, LogicExpression* source = NULL, NodeSource* node = NULL);
        virtual ~whyr_warning();
    };
}

#endif /* INCLUDE_WHYR_EXCEPTION_HPP_ */
