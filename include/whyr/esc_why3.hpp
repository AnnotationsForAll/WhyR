/*
 * esc_why3.hpp
 *
 *  Created on: Sep 15, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_ESC_WHY3_HPP_
#define INCLUDE_WHYR_ESC_WHY3_HPP_

/**
 * This file describes the functions used for ESC (Extended Static Checking) via Why3.
 */

#include "module.hpp"

#include <unordered_map>
#include <unordered_set>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    /*
     * 
     * TYPE INFORMATION DISCOVERY
     * 
     */
    
    /**
     * This represents discovered information about the module (or part of the module).
     * 
     * Call getTypeInfo on the part of the module you want to collect information about,
     * using a newly created TypeInfo.
     */
    struct TypeInfo {
        /// The module we're type checking.
        AnnotatedModule* module;
        /// All the integer types used.
        unordered_set<IntegerType*> intTypes;
        /// All the pointer types used.
        unordered_set<PointerType*> ptrTypes;
        /// All the pointer types used.
        unordered_set<Type*> floatTypes;
        /// All the array types used.
        unordered_set<ArrayType*> arrayTypes;
        /// All the struct types used.
        unordered_set<StructType*> structTypes;
        /// All the functions called by other functions.
        unordered_set<Function*> funcsCalled;
        /// All the globals used by other functions.
        unordered_set<GlobalValue*> globalsUsed;
        /// If true, a piece of code uses the 'alloca' instruction.
        bool usesAlloc = false;
        /// All the local variables in the program, including function arguments.
        unordered_set<Value*> locals;
        /// All the statepoints that need to be defined.
        unordered_set<string> statepoints;
        /// If true, a piece of code creates a block address constant. Used with indirect branches, among other things.
        bool usesBaddr = false;
    };
    
    /// Retrieves type information. See struct TypeInfo for details.
    void getTypeInfo(TypeInfo &info, Type* type);
    /// Retrieves type information. See struct TypeInfo for details.
    void getTypeInfo(TypeInfo &info, AnnotatedFunction* func, Instruction* inst);
    /// Retrieves type information. See struct TypeInfo for details.
    void getTypeInfo(TypeInfo &info, AnnotatedFunction* func);
    /// Retrieves type information. See struct TypeInfo for details.
    void getTypeInfo(TypeInfo &info, AnnotatedModule* module);
    /// This copies all data from 'other' into 'info', appending to lists as needed.
    void getTypeInfo(TypeInfo &info, TypeInfo &other);
    
    /*
     * 
     * NAME MANGLING
     * 
     */
    
    /**
     * This function performs name mangling on LLVM identifiers, putting them in a Why3-safe format.
     * When you have a Why3 name based on an LLVM name, ensure the following:
     *    1. You give the variable some sort of prefix (like val_, global_, etc).
     *    2. You run the user-supplied portion of the name through this function.
     */
    string getWhy3SafeName(string name);
    /**
     * Sometimes, we need an identifier number for a unique, yet nameless entity.
     * getTempName provides such functionality, by mapping pointers to an incrementing counter.
     * It is garunteed to produce the same ID string for the same pointer.
     */
    string getTempName(void* p);
    /**
     * Returns the theory name of a LLVM type.
     * For example, LLVM's "i32*" type is inside the "I32P" theory.
     * 
     * Be sure to "use import" any theories you reference!
     */
    string getWhy3TheoryName(Type* type);
    /**
     * Returns the type name of a LLVM type.
     * For example, LLVM's "i32*" type is called "i32p" in Why3.
     */
    string getWhy3TypeName(Type* type);
    /**
     * A combination of getWhy3TheoryName and getWhy3TypeName.
     * For example, LLVM's "i32*" type can be unambiguously addressed as "I32P.i32p" in Why3.
     * 
     * Be sure to "use import" any theories you reference!
     */
    string getWhy3FullName(Type* type);
    /**
     * Returns the name of the theory corresponding to a LLVM function.
     * 
     * Be sure to "use import" any theories you reference!
     */
    string getWhy3TheoryName(AnnotatedFunction* func);
    /**
     * Returns the name of a constant representing a local variable.
     */
    string getWhy3VarName(Value* var);
    /**
     * When a function is called in the model, the argument names are mangled.
     * Use this function to retrieve the constant corresponding to an argument of the function you are calling.
     * 'callee' is the callee theory name, which is the value of Why3Data.calleeTheoryName.
     */
    string getWhy3ArgName(string callee, Value* var);
    /**
     * Returns the name of a block expression corresponding to the execution of the specified basic block.
     */
    string getWhy3BlockName(AnnotatedFunction* func, BasicBlock* block);
    /**
     * Returns the name of the predicate representing the effects of the given instruction.
     */
    string getWhy3StatementName(AnnotatedFunction* func, Instruction* inst);
    /**
     * Returns the Why3 name of a logic-local variable.
     */
    string getWhy3LocalName(LogicLocal* local);
    /**
     * Returns the name of the statepoint constant at the beginning of the block. Branch instructions set this to their current statepoint.
     */
    string getWhy3StatepointBeforeBlock(AnnotatedFunction* func, BasicBlock* block);
    /**
     * Returns the name of the statepoint constant before this instruction finishes executing.
     * To get the statepoint after executing (i.e, you're an instruction like store or alloca), call this with "inst->getNextNode()" rather than "inst".
     */
    string getWhy3StatepointBefore(AnnotatedFunction* func, Instruction* inst);
    /**
     * Returns the constant name of a global variable.
     */
    string getWhy3GlobalName(GlobalVariable* global);
    /**
     * Returns the constant name of a global variable.
     */
    string getWhy3GlobalName(GlobalValue* global);
    /**
     * Returns the name of a field in a struct.
     * Be sure to add the preceding '.' before adding this to the output stream!
     * This function requires the module, because it might rely on debug info to make names.
     */
    string getWhy3StructFieldName(AnnotatedModule* module, StructType* type, unsigned index);
    
    /*
     * 
     * WHY3 GENERATION
     * 
     */
    
    /**
     * Adds the imports needed to ensure no errors due to missing theories can occur.
     * Use getTypeInfo on the TypeInfo passed in first.
     */
    void addImports(ostream &out, NodeSource* source, TypeInfo &info);
    /**
     * Adds the imports needed to ensure no errors due to missing theories can occur.
     * This version uses Why3Data, which pays attention to extra LogicExpression imports, among other things.
     */
    void addImports(ostream &out, Why3Data &data);
    /**
     * Adds a constant of a LLVM integer type.
     * Note that the number you pass in is encoded as a string - this allows you to pass in arbitrary-precision numbers.
     */
    void addLLVMIntConstant(ostream &out, AnnotatedModule* module, Type* type, string number);
    /**
     * Adds a constant of a LLVM floating type.
     * Note that the number you pass in is encoded as a string - this allows you to pass in arbitrary-precision numbers.
     * Make sure the number you pass in has a decimal in it!
     */
    void addLLVMFloatConstant(ostream &out, AnnotatedModule* module, Type* type, string number);
    /**
     * Adds an operand expression, either a constant or a local variable.
     * Pass it in an Instruction* to add the return value variable to the stream.
     */
    void addOperand(ostream &out, AnnotatedModule* module, Value* operand, AnnotatedFunction* func = NULL);
    /**
     * Adds a basic block definition to a function theory.
     */
    void addBlock(ostream &out, AnnotatedFunction* func, BasicBlock* block, LogicExpression* goalExpr = NULL, Instruction* goalInst = NULL);
    /**
     * All branches need to imply the contents of their successor's phi instructions.
     * Call this function to add any implications necessary before implying the block predicate.
     */
    void addWhy3PhiImplications(ostream &out, AnnotatedFunction* func, BasicBlock* block, BasicBlock* succ);
    /**
     * Adds the definition of an instruction to a basic block definition.
     */
    void addInstruction(ostream &out, AnnotatedFunction* func, Instruction* inst, LogicExpression* goalExpr = NULL, Instruction* goalInst = NULL);
    /**
     * Adds a theory for modelling a function. No goals are generated.
     */
    void addFunction(ostream &out, AnnotatedFunction* func);
    /**
     * Adds a theory for a goal of a function. goalExpr is the clause (ensures, assert, etc.) to prove.
     * All other clauses that are not goalExpr will be added as assumptions instead.
     * if goalExpr = NULL, no goals are generated, and the result is similar to addFunction.
     * goalInst restricts the assertion to one specific instance, if not NULL.
     */
    void addGoal(ostream &out, AnnotatedFunction* func, string theoryName, LogicExpression* goalExpr, Instruction* goalInst);
    /**
     * Adds the base theory for all integer types.
     * This is not in the common header because it depends on the settings of the module.
     */
    void addCommonIntType(ostream &out, AnnotatedModule* module);
    /**
     * Adds a theory for an integer type.
     */
    void addIntType(ostream &out, AnnotatedModule* module, IntegerType* type);
    /**
     * Adds the base theory for all pointer types.
     * This is not in the common header because it depends on the LLVM data layout to determine its size.
     */
    void addCommonPtrType(ostream &out, AnnotatedModule* module);
    /**
     * Adds a theory for pointer types. Be sure to call addCommonPtrType before any of these!
     */
    void addPtrType(ostream &out, AnnotatedModule* module, PointerType* type);
    /**
     * Adds the base theory for all floating types.
     * This is not in the common header because it depends on the settings of the module.
     */
    void addCommonFloatType(ostream &out, AnnotatedModule* module);
    /**
     * Adds a theory for floating types.
     */
    void addFloatType(ostream &out, AnnotatedModule* module, Type* type);
    /**
     * Adds the base theory for all array types.
     */
    void addCommonArrayType(ostream &out, AnnotatedModule* module);
    /**
     * Adds a theory for an array type.
     */
    void addArrayType(ostream &out, AnnotatedModule* module, ArrayType* type);
    /**
     * Adds the base theory for all struct types.
     */
    void addCommonStructType(ostream &out, AnnotatedModule* module);
    /**
     * Adds a theory for a struct type.
     */
    void addStructType(ostream &out, AnnotatedModule* module, StructType* type);
    /**
     * Adds a theory for a derived type.
     * This function calls addArrayType, addPtrType, etc.
     */
    void addDerivedType(ostream &out, AnnotatedModule* module, Type* type);
    /**
     * Adds a theory for block address / indirectbr operations.
     */
    void addBlockAddressType(ostream &out, AnnotatedModule* module);
    /**
     * Adds the Globals theory, which initializes all the globals in the program.
     * This adds the useful constants any_state and init_state.
     */
    void addGlobals(ostream &out, AnnotatedModule* module);
    /**
     * Adds the memory model theories all functions need, as set in the WhyRSettings.
     * If only pointer types need parts of your model's theories, add them to the output in addCommonPtrType instead.
     */
    void addMemoryModel(ostream &out, AnnotatedModule* module);
    /**
     * Pass this function in an empty list for types.
     * This function will place in types all the derived types that, when defined in Why3 in that order, will successfully depend on one another.
     * TODO: In the future, struct types will introduce the possibility of recursion, as is the problem with functions.
     */
    void getCorrectDerivedTypeOrder(list<Type*> &types, TypeInfo &info, AnnotatedModule* mod);
    /**
     * Pass this function in an empty list for funcs.
     * This function will place in funcs all the functions that, when defined in Why3 in that order, will successfully depend on one another.
     * TODO: What about recursion and mutual recursion?
     */
    void getCorrectFunctionOrder(list<AnnotatedFunction*> &funcs, AnnotatedModule* mod);
    /**
     * This adds all the goals found in the program.
     */
    void addGoals(ostream &out, AnnotatedModule* module);
    /**
     * Writes to the output stream the Why3 corresponding to the module given.
     * When run through "why3 prove", this will attempt to verify the assertions given in the LLVM.
     */
    void generateWhy3(ostream &out, AnnotatedModule* module);
}

#endif /* INCLUDE_WHYR_ESC_WHY3_HPP_ */
