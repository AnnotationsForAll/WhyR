/*
 * logic.hpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_LOGIC_HPP_
#define INCLUDE_WHYR_LOGIC_HPP_

/**
 * This file contains the core elements of WhyR annotations- Expressions and types.
 */

#include "whyr.hpp"

#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    class AnnotatedFunction; class AnnotatedInstruction; struct TypeInfo; class AnnotatedModule;
    class LogicType; class LogicExpression;
    
    struct LogicLocal {
        string name;
        LogicType* type;
    };
    
    struct LogicDebugInfo {
        const char* file = NULL;
        const char* line = NULL;
        const char* col1 = NULL;
        const char* col2 = NULL;
    };
    
    /**
     * This represents the source location of a WhyR metadata node.
     * Expressions like \result need this while parsing, to know what function they come from.
     * 
     * This struct does not own any of its members. It will not free them on destruction.
     */
    struct NodeSource {
        /// The function the expression came from.
        AnnotatedFunction* func;
        /// The LLVM instruction the expression came from. May be NULL if this is from a function contract.
        Instruction* inst;
        /// The raw metadata node that created the expression. May be NULL if we're not in the parsing phase.
        Metadata* metadata;
        /// The list of logic-locals. The last element on a value of this map is the current logic-local with that name.
        unordered_map<string, list<LogicLocal*>> logicLocals;
        /// Specifies where this logic node came from in a source file. All the fields may be NULL.
        list<LogicDebugInfo> debugInfo;
        /// A label for this expression. Helps in Why3 generation and error generation.
        const char* label = NULL;
        
        NodeSource(AnnotatedFunction* func, Instruction* inst = NULL, Metadata* metadata = NULL);
        NodeSource(NodeSource* other);
    };
    
    /**
     * This structure contains useful data to pass to logical expressions that are being converted to Why3.
     * See "esc_why3.hpp" for details.
     */
    struct Why3Data {
        /// The module this code originates from. Should NOT be null.
        AnnotatedModule* module;
        /// The source of the call.
        NodeSource* source;
        /// This is for any imports that cannot be described in the TypeInfo.
        unordered_set<string> importsNeeded;
        /// This is combined with the existing type info to add more imports, etc. Should NOT be null.
        TypeInfo* info;
        /// If this is occurring as part of a function call instruction, this will be non-NULL.
        const char* calleeTheoryName = NULL;
        /// The current statepoint, for use in memory-access expressions.
        string statepoint = "entry_state";
    };
    
    /**
     * A logical WhyR type. This can directly correspond to a LLVM type, a Why3 type, or something entirely new.
     */
    class LogicType {
    protected:
        NodeSource* source;
    public:
        /// This id number has to be public due to LLVM's constraints. Not for user use! Use isa and cast instead.
        int id;
        LogicType(NodeSource* source);
        virtual ~LogicType();
        
        /**
         * Returns the source used to create this type. Can be NULL.
         */
        NodeSource* getSource();
        /**
         * Sets the source of this expression. Can be NULL.
         */
        void setSource(NodeSource* source);
        /// Converts the type to a human-readable format. This is NOT why3 format!
        virtual string toString();
        /**
         * Returns whether or not this type is exactly equivalent to another type.
         * Note that you CANNOT compare pointers of LogicType s to check for equality!
         */
        virtual bool equals(LogicType* other);
        /**
         * This returns the Why3 full theory name of the type, appending it to the stream.
         */
        virtual void toWhy3(ostream &out, Why3Data &data);
        /**
         * We implement this function so we can use LLVM's casting functions on LogicType s,
         * which are isa, case, and dyn_cast. See the LLVm documentation for how to use those.
         * This has to be public, but is NOT intended for public use!
         */
        static bool classof(const LogicType* type);
    };
    
    /**
     * A WhyR expression. This is separate from Why3 expressions, but are translated to Why3 axioms.
     */
    class LogicExpression {
    protected:
        NodeSource* source;
    public:
        /// This id number has to be public due to LLVM's constraints. Not for user use! Use isa and cast instead.
        int id;
        LogicExpression(NodeSource* source = NULL);
        virtual ~LogicExpression();
        
        /**
         * Returns the source used to create this expression. Can be NULL.
         */
        NodeSource* getSource();
        /**
         * Sets the source of this expression. Can be NULL.
         */
        void setSource(NodeSource* source);
        /// Converts the type to a human-readable format. This is NOT why3 format!
        virtual string toString();
        /**
         * Returns the type of this expression. All expressions have a type.
         * 
         * This object owns the resulting LogicType. It will free it on deletion.
         */
        virtual LogicType* returnType();
        /**
         * Some expressions have constraints on the types of thier members. For example, eq requires both sides be the same type.
         * Calling this function will verify the types match for this expression and any subexpressions.
         * If the types do not match, a type_exception will be thrown.
         */
        virtual void checkTypes();
        /**
         * This converts the expression to Why3, appending it to the stream.
         */
        virtual void toWhy3(ostream &out, Why3Data &data);
        /**
         * We implement this function so we can use LLVM's casting functions on LogicType s,
         * which are isa, case, and dyn_cast. See the LLVm documentation for how to use those.
         * This has to be public, but is NOT intended for public use!
         */
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This class represents the metadata parsing mechanism. It takes metadata nodes and converts them to WhyR logic expressions.
     * Implement a subclass of this to add a new parser for a new expression.
     * See "parser.cpp" for existing parsers.
     */
    class ExpressionParser {
    protected:
        /**
         * In a parser function, call this to require a minimum number of arguments.
         * If there are not enough arguments in the node, it throws a syntax_exception.
         */
        static void requireMinArgs(MDNode* &node, const char* &exprName, NodeSource* source, unsigned nArgs);
        /**
         * In a parser function, call this to require a maximum number of arguments.
         * If there are too many arguments in the node, it throws a syntax_exception.
         */
        static void requireMaxArgs(MDNode* &node, const char* &exprName, NodeSource* source, unsigned nArgs);
    public:
        /**
         * Returns the map of expression names to expression parsers.
         * This returns the field contained in "parser.cpp".
         * To add a new parser at startup, edit the initializer in that file.
         */
        static map<string,ExpressionParser*>* getExpressionParsers();
        /**
         * Call this function to convert a metadata into a WhyR expression.
         * This function returns a non-null value representing the expression, or throws an exception.
         * 
         * The caller owns the resulting LogicExpression. Free it when you are done.
         */
        static LogicExpression* parseMetadata(Metadata* node, NodeSource* source);
        
        ExpressionParser();
        virtual ~ExpressionParser();
        
        /**
         * Override this to provide parser functionality. You are given the registered name of the expression,
         * the node to parse, and other source data.
         * Note that operand 0 of the MDNode given is the expression name string; effectively, your arguments are numbered starting at 1!
         * Do not return null. If there is an issue, throw an exception.
         * 
         * The caller owns the resulting LogicExpression. Free it when you are done.
         */
        virtual LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) = 0;
    };
}


#endif /* INCLUDE_WHYR_LOGIC_HPP_ */
