/*
 * types.hpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_TYPES_HPP_
#define INCLUDE_WHYR_TYPES_HPP_

/**
 * This file contains all the subclasses of LogicType, located in "logic.hpp".
 * 
 * Except where noted, the objects themselves own thier unique members, and will free them on deletion.
 */

#include "logic.hpp"

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    /**
     * An enum for all the logical types, used as part of LLVM's classof system.
     * Users have no use for this value. Use isa and cast instead.
     */
    enum LogicTypes {
        LOGIC_TYPE_LLVM,
        LOGIC_TYPE_BOOL,
        LOGIC_TYPE_SET,
        LOGIC_TYPE_TYPE,
        LOGIC_TYPE_INT,
        LOGIC_TYPE_REAL,
    };
    
    /**
     * This type corresponds to a given LLVM type.
     */
    class LogicTypeLLVM : public LogicType {
    protected:
        Type* type;
    public:
        LogicTypeLLVM(Type* type, NodeSource* source = NULL);
        /**
         * The LLVM Module owns the resulting Constant. It will free it on the Module's deletion.
         */
        Type* getType();
        
        virtual ~LogicTypeLLVM();
        virtual string toString();
        virtual bool equals(LogicType* other);
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicType* type);
    };
    
    /**
     * This is a logical boolean type. NOT a 1-bit bitvector type, unlike LLVM booleans!
     */
    class LogicTypeBool : public LogicType {
    public:
        LogicTypeBool(NodeSource* source = NULL);
        
        virtual ~LogicTypeBool();
        virtual string toString();
        virtual bool equals(LogicType* other);
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicType* type);
    };
    
    /**
     * This is a type for a set of values. Used in assigns clauses, for example.
     */
    class LogicTypeSet : public LogicType {
    protected:
        LogicType* type;
    public:
        LogicTypeSet(LogicType* type, NodeSource* source = NULL);
        LogicType* getType();
        
        virtual ~LogicTypeSet();
        virtual string toString();
        virtual bool equals(LogicType* other);
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicType* type);
    };
    
    /**
     * This is the type of type expressions.
     * This is used to parse type specifications while still using the expression parser.
     * Used with "typeof" expressions, etc.
     */
    class LogicTypeType : public LogicType {
    protected:
        LogicType* type;
    public:
        LogicTypeType(LogicType* type, NodeSource* source = NULL);
        LogicType* getType();
        
        virtual ~LogicTypeType();
        virtual string toString();
        virtual bool equals(LogicType* other);
        
        static bool classof(const LogicType* type);
    };
    
    /**
     * This is a logical integer type. NOT a bitvector type, unlike LLVM ints!
     */
    class LogicTypeInt : public LogicType {
    public:
        LogicTypeInt(NodeSource* source = NULL);
        
        virtual ~LogicTypeInt();
        virtual string toString();
        virtual bool equals(LogicType* other);
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicType* type);
    };
    
    /**
     * This is a real number type. NOT a floating-point type, unlike LLVM floats!
     */
    class LogicTypeReal : public LogicType {
    public:
        LogicTypeReal(NodeSource* source = NULL);
        
        virtual ~LogicTypeReal();
        virtual string toString();
        virtual bool equals(LogicType* other);
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicType* type);
    };
}

#endif /* INCLUDE_WHYR_TYPES_HPP_ */
