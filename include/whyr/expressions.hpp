/*
 * expressions.hpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_EXPRESSIONS_HPP_
#define INCLUDE_WHYR_EXPRESSIONS_HPP_

/**
 * This file contains all the subclasses of LogicException, located in "logic.hpp".
 * 
 * Except where noted, the objects themselves own thier unique members, and will free them on deletion.
 */

#include "logic.hpp"
#include "types.hpp"

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    /**
     * An enum for all the logical expression kinds, used as part of LLVM's classof system.
     * Users have no use for this value. Use isa and cast instead.
     */
    enum LogicExpressions {
        LOGIC_EXPR_EQ,
        LOGIC_EXPR_LLVM,
        LOGIC_EXPR_SET,
        LOGIC_EXPR_TYPE,
        LOGIC_EXPR_RESULT,
        LOGIC_EXPR_ARG,
        LOGIC_EXPR_VAR,
        LOGIC_EXPR_BOOL_CONST,
        LOGIC_EXPR_BOOL,
        LOGIC_EXPR_BIN_BOOL,
        LOGIC_EXPR_NOT,
        LOGIC_EXPR_INT_CONST,
        LOGIC_EXPR_BIN_MATH,
        LOGIC_EXPR_LOCAL,
        LOGIC_EXPR_QUANT,
        LOGIC_EXPR_NEG,
        LOGIC_EXPR_IFTE,
        LOGIC_EXPR_LET,
        LOGIC_EXPR_BIN_BITS,
        LOGIC_EXPR_BIN_COMP,
        LOGIC_EXPR_BIN_COMP_LLVM,
        LOGIC_EXPR_BIT_NOT,
        LOGIC_EXPR_BIN_SHIFT,
        LOGIC_EXPR_CAST_LLVM_LLVM,
        LOGIC_EXPR_CAST_INT_LLVM,
        LOGIC_EXPR_CAST_LLVM_INT,
        LOGIC_EXPR_REAL_CONST,
        LOGIC_EXPR_CAST_FLOAT_REAL,
        LOGIC_EXPR_CAST_REAL_FLOAT,
        LOGIC_EXPR_CAST_REAL_INT,
        LOGIC_EXPR_CAST_INT_REAL,
        LOGIC_EXPR_BIN_COMP_FLOAT,
        LOGIC_EXPR_LOAD,
        LOGIC_EXPR_GET_INDEX,
        LOGIC_EXPR_LLVM_ARRAY_CONST,
        LOGIC_EXPR_SET_INDEX,
        LOGIC_EXPR_CAST_PTR_INT,
        LOGIC_EXPR_CAST_INT_PTR,
        LOGIC_EXPR_CAST_PTR_PTR,
        LOGIC_EXPR_GEP,
        LOGIC_EXPR_OPERAND,
        LOGIC_EXPR_SPECIAL_LLVM_CONST,
        LOGIC_EXPR_BADDR,
        LOGIC_EXPR_LLVM_STRUCT_CONST,
        LOGIC_EXPR_LLVM_VECTOR_CONST,
        LOGIC_EXPR_IN_SET,
        LOGIC_EXPR_SUBSET,
    };
    
    /**
     * A binary equals expression.
     */
    class LogicExpressionEquals : public LogicExpression {
    protected:
        LogicExpression* lhs;
        LogicExpression* rhs;
        bool negated;
    public:
        LogicExpressionEquals(LogicExpression* lhs, LogicExpression* rhs, bool negated = false, NodeSource* source = NULL);
        LogicExpression* getLeft();
        LogicExpression* getRight();
        bool isNegated();
        
        virtual ~LogicExpressionEquals();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is the expression type of any LLVM constants directly in metadata nodes.
     * All LogicExpressionLLVMConstant s have a return type of LogicTypeLLVM.
     */
    class LogicExpressionLLVMConstant : public LogicExpression {
    protected:
        Constant* value;
    public:
        LogicExpressionLLVMConstant(Constant* value,NodeSource* source = NULL);
        /**
         * The LLVM Module owns the resulting Constant. It will free it on the Module's deletion.
         */
        Constant* getValue();
        
        virtual ~LogicExpressionLLVMConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is the set constant expression. It has a base type, and all elements must be of that type.
     */
    class LogicExpressionCreateSet : public LogicExpression {
    protected:
        LogicType* baseType;
        LogicTypeSet setType;
        list<LogicExpression*> elems;
    public:
        LogicExpressionCreateSet(LogicType* baseType, list<LogicExpression*> &elems, NodeSource* source = NULL);
        list<LogicExpression*>* getElements();
        LogicType* getBaseType();
        
        virtual ~LogicExpressionCreateSet();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a type variable.
     */
    class LogicExpressionConstantType : public LogicExpression {
    protected:
        LogicTypeType* type;
    public:
        LogicExpressionConstantType(LogicTypeType* type, NodeSource* source = NULL);
        LogicTypeType* getType();
        
        virtual ~LogicExpressionConstantType();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents the result of the current function's execution.
     * Note that it has no meaning outside of function contracts.
     */
    class LogicExpressionResult : public LogicExpression {
    protected:
        LogicType* type;
    public:
        LogicExpressionResult(LogicType* type, NodeSource* source = NULL);
        LogicType* getType();
        
        virtual ~LogicExpressionResult();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents an argument passed into a function.
     * Note that it has no meaning outside of function contracts.
     */
    class LogicExpressionArgument : public LogicExpression {
    protected:
        Function* function;
        Argument* arg = NULL;
        string name;
        LogicType* retType;
    public:
        LogicExpressionArgument(Function* function, string name, NodeSource* source = NULL);
        string getName();
        
        virtual ~LogicExpressionArgument();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a local variable.
     * Note that this expression has no meaning in function contracts.
     */
    class LogicExpressionVariable : public LogicExpression {
    protected:
        Function* function;
        Value* arg = NULL;
        string name;
        LogicType* retType;
    public:
        LogicExpressionVariable(Function* function, string name, NodeSource* source = NULL);
        string getName();
        
        virtual ~LogicExpressionVariable();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a constant Boolean value- true or false.
     */
    class LogicExpressionBooleanConstant : public LogicExpression {
    protected:
        bool value;
    public:
        LogicExpressionBooleanConstant(bool value, NodeSource* source = NULL);
        bool getValue();
        
        virtual ~LogicExpressionBooleanConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression converts an LLVM i1 type into a logical boolean type.
     */
    class LogicExpressionBoolean : public LogicExpression {
    protected:
        LogicExpression* value;
    public:
        LogicExpressionBoolean(LogicExpression* value, NodeSource* source = NULL);
        LogicExpression* getValue();
        
        virtual ~LogicExpressionBoolean();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression is a binary boolean operator: And, or, implies, etc.
     */
    class LogicExpressionBinaryBoolean : public LogicExpression {
    public:
        enum BinaryBooleanOp {
            OP_AND,
            OP_OR,
            OP_IMPLIES,
            OP_BIDIR_IMPLIES,
        };
    protected:
        BinaryBooleanOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryBoolean(BinaryBooleanOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryBooleanOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryBoolean();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression is the boolean NOT operator.
     */
    class LogicExpressionNot : public LogicExpression {
    protected:
        LogicExpression* rhs;
    public:
        LogicExpressionNot(LogicExpression* rhs, NodeSource* source = NULL);
        LogicExpression* getValue();
        
        virtual ~LogicExpressionNot();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a constant integer value.
     */
    class LogicExpressionIntegerConstant : public LogicExpression {
    protected:
        string value;
    public:
        LogicExpressionIntegerConstant(string value, NodeSource* source = NULL);
        string getValue();
        
        virtual ~LogicExpressionIntegerConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that can act on any numeric type.
     * Note that this does not include operations specific to LLVM integers, such as bitwise operations!
     */
    class LogicExpressionBinaryMath : public LogicExpression {
    public:
        enum BinaryMathOp {
            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_MOD,
            OP_REM,
        };
    protected:
        BinaryMathOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryMath(BinaryMathOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryMathOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryMath();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a logic-local, used with quantifiers and let statements.
     * The LogicLocal in question has to exist in the NodeSource when you create this object!
     */
    class LogicExpressionLocal : public LogicExpression {
    protected:
        string name;
        LogicLocal* local;
    public:
        LogicExpressionLocal(string name, NodeSource* source = NULL);
        string getName();
        
        virtual ~LogicExpressionLocal();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a quantifier- A forall or exists.
     * This does NOT add the locals it gets to the scope; add the locals to the scope before parsing the expr passed in.
     * 
     * Note that this class claims ownership of the arguments passed in. It will delete them when done.
     */
    class LogicExpressionQuantifier : public LogicExpression {
    protected:
        bool forall;
        list<LogicLocal*>* locals;
        LogicExpression* expr;
    public:
        LogicExpressionQuantifier(bool forall, list<LogicLocal*>* locals, LogicExpression* expr, NodeSource* source = NULL);
        bool isForall();
        list<LogicLocal*>* getLocals();
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionQuantifier();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is the unary negation operator.
     */
    class LogicExpressionNegate : public LogicExpression {
    protected:
        LogicExpression* rhs;
    public:
        LogicExpressionNegate(LogicExpression* rhs, NodeSource* source = NULL);
        LogicExpression* getValue();
        
        virtual ~LogicExpressionNegate();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is the if/then/else operator.
     */
    class LogicExpressionConditional : public LogicExpression {
    protected:
        LogicExpression* condition;
        LogicExpression* ifTrue;
        LogicExpression* ifFalse;
        LogicType* retType;
    public:
        LogicExpressionConditional(LogicExpression* condition, LogicExpression* ifTrue, LogicExpression* ifFalse, NodeSource* source = NULL);
        LogicExpression* getCondition();
        LogicExpression* getIfTrue();
        LogicExpression* getIfFalse();
        
        virtual ~LogicExpressionConditional();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a let statement.
     * This does NOT add the locals it gets to the scope; add the locals to the scope before parsing the expr passed in.
     * 
     * Note that this class claims ownership of the arguments passed in. It will delete them when done.
     */
    class LogicExpressionLet : public LogicExpression {
    protected:
        list<pair<LogicLocal*, LogicExpression*>*>* locals;
        LogicExpression* expr;
    public:
        LogicExpressionLet(list<pair<LogicLocal*, LogicExpression*>*>* locals, LogicExpression* expr, NodeSource* source = NULL);
        bool isForall();
        list<pair<LogicLocal*, LogicExpression*>*>* getLocals();
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionLet();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that can act on bitwise types only.
     * Note that this does not include operations that can also act on logical integers!
     */
    class LogicExpressionBinaryBits : public LogicExpression {
    public:
        enum BinaryBitsOp {
            OP_SDIV,
            OP_UDIV,
            OP_SMOD,
            OP_UMOD,
            OP_SREM,
            OP_UREM,
            OP_AND,
            OP_OR,
            OP_XOR,
        };
    protected:
        BinaryBitsOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryBits(BinaryBitsOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryBitsOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryBits();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that acts on logical integers or reals, and produces a boolean.
     */
    class LogicExpressionBinaryCompare : public LogicExpression {
    public:
        enum BinaryCompareOp {
            OP_GT,
            OP_GE,
            OP_LT,
            OP_LE,
        };
    protected:
        BinaryCompareOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryCompare(BinaryCompareOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryCompareOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryCompare();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that acts on LLVM integers and produces a boolean.
     */
    class LogicExpressionBinaryCompareLLVM : public LogicExpression {
    public:
        enum BinaryCompareLLVMOp {
            OP_SGT,
            OP_SGE,
            OP_SLT,
            OP_SLE,
            OP_UGT,
            OP_UGE,
            OP_ULT,
            OP_ULE,
        };
    protected:
        BinaryCompareLLVMOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryCompareLLVM(BinaryCompareLLVMOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryCompareLLVMOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryCompareLLVM();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is the bitwise NOT operator.
     */
    class LogicExpressionBitNot : public LogicExpression {
    protected:
        LogicExpression* rhs;
    public:
        LogicExpressionBitNot(LogicExpression* rhs, NodeSource* source = NULL);
        LogicExpression* getValue();
        
        virtual ~LogicExpressionBitNot();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that takes an LLVM int and a logic int, and produces an LLVM int.
     */
    class LogicExpressionBinaryShift : public LogicExpression {
    public:
        enum BinaryShiftOp {
            OP_LSHL,
            OP_LSHR,
            OP_ASHR,
        };
    protected:
        BinaryShiftOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryShift(BinaryShiftOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryShiftOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryShift();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts an LLVM int to another width of LLVM int.
     */
    class LogicExpressionLLVMIntToLLVMInt : public LogicExpression {
    public:
        enum LLVMIntToLLVMIntOp {
            OP_TRUNC,
            OP_ZEXT,
            OP_SEXT,
        };
    protected:
        LLVMIntToLLVMIntOp op;
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionLLVMIntToLLVMInt(LLVMIntToLLVMIntOp op, LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LLVMIntToLLVMIntOp getOp();
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionLLVMIntToLLVMInt();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts an LLVM int to a logical int.
     */
    class LogicExpressionLLVMIntToLogicInt : public LogicExpression {
    public:
        enum LLVMIntToLogicIntOp {
            OP_ZEXT,
            OP_SEXT,
        };
    protected:
        LLVMIntToLogicIntOp op;
        LogicExpression* expr;
    public:
        LogicExpressionLLVMIntToLogicInt(LLVMIntToLogicIntOp op, LogicExpression* expr, NodeSource* source = NULL);
        LLVMIntToLogicIntOp getOp();
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionLLVMIntToLogicInt();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts a logical int to a LLVM int.
     */
    class LogicExpressionLogicIntToLLVMInt : public LogicExpression {
    protected:
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionLogicIntToLLVMInt(LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionLogicIntToLLVMInt();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This represents a constant real value.
     */
    class LogicExpressionRealConstant : public LogicExpression {
    protected:
        string value;
    public:
        LogicExpressionRealConstant(string value, NodeSource* source = NULL);
        string getValue();
        
        virtual ~LogicExpressionRealConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts a real to a LLVM floating type.
     */
    class LogicExpressionRealToFloat : public LogicExpression {
    protected:
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionRealToFloat(LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionRealToFloat();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts a LLVM floating type to a real.
     */
    class LogicExpressionFloatToReal : public LogicExpression {
    protected:
        LogicExpression* expr;
    public:
        LogicExpressionFloatToReal(LogicExpression* expr, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionFloatToReal();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts a real to an int.
     */
    class LogicExpressionRealToInt : public LogicExpression {
    protected:
        LogicExpression* expr;
    public:
        LogicExpressionRealToInt(LogicExpression* expr, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionRealToInt();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts an int to a real.
     */
    class LogicExpressionIntToReal : public LogicExpression {
    protected:
        LogicExpression* expr;
    public:
        LogicExpressionIntToReal(LogicExpression* expr, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionIntToReal();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a binary operator that acts on LLVM floats and produces a boolean.
     */
    class LogicExpressionBinaryCompareFloat : public LogicExpression {
    public:
        enum BinaryCompareFloatOp {
            OP_OEQ,
            OP_OGT,
            OP_OGE,
            OP_OLT,
            OP_OLE,
            OP_ONE,
            OP_ORD,
            OP_UEQ,
            OP_UGT,
            OP_UGE,
            OP_ULT,
            OP_ULE,
            OP_UNE,
            OP_UNO,
        };
    protected:
        BinaryCompareFloatOp op;
        LogicExpression* lhs;
        LogicExpression* rhs;
    public:
        LogicExpressionBinaryCompareFloat(BinaryCompareFloatOp op, LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        BinaryCompareFloatOp getOp();
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionBinaryCompareFloat();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression loads a value from a pointer. expr needs to be of LLVM pointer type.
     */
    class LogicExpressionLoad : public LogicExpression {
    protected:
        LogicExpression* expr;
        PointerType* ptrType;
        LogicTypeLLVM* retType;
    public:
        LogicExpressionLoad(LogicExpression* expr, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionLoad();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is an index into an aggregate type (most likely an array or map).
     */
    class LogicExpressionGetIndex : public LogicExpression {
    protected:
        LogicExpression* lhs;
        LogicExpression* rhs;
        LogicType* retType;
    public:
        LogicExpressionGetIndex(LogicExpression* lhs, LogicExpression* rhs, NodeSource* source = NULL);
        LogicExpression* getLeft();
        LogicExpression* getRight();
        
        virtual ~LogicExpressionGetIndex();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This constructs an LLVM array out of potentially logical intermediate values.
     */
    class LogicExpressionLLVMArrayConstant : public LogicExpression {
    protected:
        list<LogicExpression*>* elements;
        Type* type;
        LogicTypeLLVM* retType;
    public:
        LogicExpressionLLVMArrayConstant(Type* type, list<LogicExpression*>* elements, NodeSource* source = NULL);
        Type* getType();
        list<LogicExpression*>* getElems();
        
        virtual ~LogicExpressionLLVMArrayConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type is a functional update of an aggregate type (most likely an array or map).
     */
    class LogicExpressionSetIndex : public LogicExpression {
    protected:
        LogicExpression* lhs;
        LogicExpression* rhs;
        LogicExpression* value;
    public:
        LogicExpressionSetIndex(LogicExpression* lhs, LogicExpression* rhs, LogicExpression* value, NodeSource* source = NULL);
        LogicExpression* getLeft();
        LogicExpression* getRight();
        LogicExpression* getValue();
        
        virtual ~LogicExpressionSetIndex();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts a pointer to an LLVM int.
     */
    class LogicExpressionPointerToInt : public LogicExpression {
    protected:
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionPointerToInt(LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionPointerToInt();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type converts an LLVM int to a pointer.
     */
    class LogicExpressionIntToPointer : public LogicExpression {
    protected:
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionIntToPointer(LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionIntToPointer();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type casts one pointer to another (a type changing no-op cast).
     */
    class LogicExpressionPointerToPointer : public LogicExpression {
    protected:
        LogicExpression* expr;
        LogicType* retType;
    public:
        LogicExpressionPointerToPointer(LogicExpression* expr, LogicType* retType, NodeSource* source = NULL);
        LogicExpression* getExpr();
        
        virtual ~LogicExpressionPointerToPointer();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression type performs a getelementptr - Finding the address of an element of a pointer.
     */
    class LogicExpressionGetElementPointer : public LogicExpression {
    protected:
        LogicExpression* expr;
        list<LogicExpression*>* elements;
        LogicType* retType;
    public:
        LogicExpressionGetElementPointer(LogicExpression* expr, list<LogicExpression*>* elements, NodeSource* source = NULL);
        LogicExpression* getExpr();
        list<LogicExpression*>* getElements();
        
        virtual ~LogicExpressionGetElementPointer();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression represents an arbitrary LLVM operand - Either a variable or a constant.
     * You cannot create this expression type via WhyR metadata or WAR. It is used internally by functions like RTE generation.
     */
    class LogicExpressionLLVMOperand : public LogicExpression {
    protected:
        Value* operand;
        LogicType* retType;
    public:
        LogicExpressionLLVMOperand(Value* operand, NodeSource* source = NULL);
        Value* getOperand();
        
        virtual ~LogicExpressionLLVMOperand();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This expression represents a special LLVM constant- Usually a maxint, minint, or other such constant.
     */
    class LogicExpressionSpecialLLVMConstant : public LogicExpression {
    public:
        enum SpecialLLVMConstOp {
            OP_MAXINT,
            OP_MININT,
            OP_MAXUINT,
            OP_MINUINT,
        };
    protected:
        SpecialLLVMConstOp op;
        LogicType* retType;
    public:
        LogicExpressionSpecialLLVMConstant(SpecialLLVMConstOp op, LogicType* retType, NodeSource* source = NULL);
        SpecialLLVMConstOp getOp();
        
        virtual ~LogicExpressionSpecialLLVMConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    /**
     * This expression represents a blockaddress constant.
     */
    class LogicExpressionBlockAddress : public LogicExpression {
    protected:
        Function* func;
        BasicBlock* block;
        LogicType* retType;
    public:
        LogicExpressionBlockAddress(Function* func, BasicBlock* block, NodeSource* source = NULL);
        Function* getFunction();
        BasicBlock* getBlock();
        
        virtual ~LogicExpressionBlockAddress();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This constructs an LLVM struct out of potentially logical intermediate values.
     */
    class LogicExpressionLLVMStructConstant : public LogicExpression {
    protected:
        list<LogicExpression*>* elements;
        LogicType* retType;
    public:
        LogicExpressionLLVMStructConstant(LogicType* type, list<LogicExpression*>* elements, NodeSource* source = NULL);
        list<LogicExpression*>* getElems();
        
        virtual ~LogicExpressionLLVMStructConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This constructs an LLVM vector out of potentially logical intermediate values.
     */
    class LogicExpressionLLVMVectorConstant : public LogicExpression {
    protected:
        list<LogicExpression*>* elements;
        Type* type;
        LogicTypeLLVM* retType;
    public:
        LogicExpressionLLVMVectorConstant(Type* type, list<LogicExpression*>* elements, NodeSource* source = NULL);
        Type* getType();
        list<LogicExpression*>* getElems();
        
        virtual ~LogicExpressionLLVMVectorConstant();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is a boolean operator that takes a set and an element, and checks if that element is in the set.
     */
    class LogicExpressionInSet : public LogicExpression {
    protected:
        LogicExpression* setExpr;
        LogicExpression* itemExpr;
    public:
        LogicExpressionInSet(LogicExpression* setExpr, LogicExpression* itemExpr, NodeSource* source = NULL);
        LogicExpression* getSetExpr();
        LogicExpression* getItemExpr();
        
        virtual ~LogicExpressionInSet();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
    
    /**
     * This is a boolean operator that takes a set and an element, and checks if that element is in the set.
     */
    class LogicExpressionSubset : public LogicExpression {
    protected:
        LogicExpression* subExpr;
        LogicExpression* superExpr;
    public:
        LogicExpressionSubset(LogicExpression* subExpr, LogicExpression* superExpr, NodeSource* source = NULL);
        LogicExpression* getSubExpr();
        LogicExpression* getSuperExpr();
        
        virtual ~LogicExpressionSubset();
        virtual string toString();
        virtual LogicType* returnType();
        virtual void checkTypes();
        virtual void toWhy3(ostream &out, Why3Data &data);
        
        static bool classof(const LogicExpression* expr);
    };
}

#endif /* INCLUDE_WHYR_EXPRESSIONS_HPP_ */
