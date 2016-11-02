/*
 * module.hpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_MODULE_HPP_
#define INCLUDE_WHYR_MODULE_HPP_

/**
 * This represents annotated versions of LLVM constructs.
 * This includes modules, functions, and instructions enhanced with WhyR annotations
 */

#include "logic.hpp"

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    class AnnotatedFunction;
    class AnnotatedModule;
    class AnnotatedInstruction;
    
    /**
     * This represents a single LLVM instruction, with added WhyR annotations.
     */
    class AnnotatedInstruction {
    protected:
        AnnotatedFunction* function;
        Instruction* llvm;
        LogicExpression* assume = NULL;
        LogicExpression* assert = NULL;
        const char* label = NULL;
    public:
        AnnotatedInstruction(AnnotatedFunction* function, Instruction* llvm);
        ~AnnotatedInstruction();
        
        /**
         * Constructing an AnnotatedInstruction does not annotate it.
         * Call this function so it can parse any attached nodes, such as assert clauses or assume clauses.
         */
        void annotate();
        /**
         * Returns the Instruction used to create this AnnotatedInstruction.
         * 
         * The LLVM Module owns the resulting Instruction. It will free it on the Module's deletion.
         */
        Instruction* rawIR();
        /**
         * Returns the annotated function this instruction is part of.
         * 
         * AnnotatedModule s own AnnotatedFunction s. Do not free this return value.
         */
        AnnotatedFunction* getFunction();
        /**
         * Returns the assume clause attached to this instruction, or NULL if it has none (or annotate() has not been called yet).
         * 
         * This object owns the resulting LogicExpression. It will free it on deletion.
         */
        LogicExpression* getAssumeClause();
        /**
         * Returns the assert clause attached to this instruction, or NULL if it has none (or annotate() has not been called yet).
         * 
         * This object owns the resulting LogicExpression. It will free it on deletion.
         */
        LogicExpression* getAssertClause();
        /**
         * Sets the assume clause of this instruction.
         * 
         * This object takes ownership of the LogicExpression passed in.
         * It returns the old value of the clause. Whoever calls the function claims ownership of this value.
         */
        LogicExpression* setAssumeClause(LogicExpression* expr);
        /**
         * Sets the assert clause of this instruction.
         * 
         * This object takes ownership of the LogicExpression passed in.
         * It returns the old value of the clause. Whoever calls the function claims ownership of this value.
         */
        LogicExpression* setAssertClause(LogicExpression* expr);
        /**
         * Returns the string label of this instruction.
         * In LLVM, only the beginning of basic blocks can be labeled. This adds another labeling mechanism,
         * so annotations can specify specific program points to compute values at.
         * 
         * This objects own the resulting string. It will free it on deletion.
         */
        const char* getLabel();
        /**
         * Returns true if the given Instruction has WhyR annotations attached to it.
         * If true, it should be parsed into an AnnotatedInstruction. If false, it should be left alone.
         */
        static bool isAnnotated(Instruction* inst);
    };
    
    /**
     * This represents a LLVM function, with added WhyR annotations.
     */
    class AnnotatedFunction {
    protected:
        AnnotatedModule* module;
        Function* llvm;
        LogicExpression* requires = NULL;
        LogicExpression* ensures = NULL;
        bool hasAssgins = false;
        list<LogicExpression*> assigns;
        list<AnnotatedInstruction*> annotatedInsts;
    public:
        AnnotatedFunction(AnnotatedModule* module, Function* llvm);
        ~AnnotatedFunction();
        
        /**
         * Constructing an AnnotatedFunction does not annotate it.
         * Call this function so it can parse any attached nodes, such as requires clauses or ensures clauses.
         */
        void annotate();
        /**
         * Returns the Function used to create this AnnotatedFunction.
         * 
         * The LLVM Module owns the resulting Function. It will free it on the Module's deletion.
         */
        Function* rawIR();
        /**
         * Returns the annotated module this function is part of.
         * 
         * AnnotatedModule s own AnnotatedFunction s. Do not free this return value.
         */
        AnnotatedModule* getModule();
        /**
         * Returns the requires clause attached to this function, or NULL if it has none (or annotate() has not been called yet).
         * 
         * This object owns the resulting LogicExpression. It will free it on deletion.
         */
        LogicExpression* getRequiresClause();
        /**
         * Returns the ensures clause attached to this function, or NULL if it has none (or annotate() has not been called yet).
         * 
         * This object owns the resulting LogicExpression. It will free it on deletion.
         */
        LogicExpression* getEnsuresClause();
        /**
         * Returns the list of memory locations that this function is allowed to assign to.
         * If it is NULL, this function was not given an assigns clause.
         * 
         * This object owns the resulting list, and all elements. It will free them on deletion.
         */
        list<LogicExpression*>* getAssignsLocations();
        /**
         * Returns the list of instructions that have annotations added to them. It will not be NULL.
         * Note that this is not all the instructions in the function, because not all instructions need to have annotations.
         * 
         * This object owns the resulting list, and all elements. It will free them on deletion.
         */
        list<AnnotatedInstruction*>* getAnnotatedInstructions();
        /**
         * Call this function to find the AnnotatedInstruction corresponding to an Instruction.
         * It returns the AnnotatedInstruction representing the Instruction, or NULL if it is not annotated.
         * 
         * This object owns the resulting AnnotatedInstruction. It will free it on deletion.
         */
        AnnotatedInstruction* getAnnotatedInstruction(Instruction* inst);
    };
    
    /**
     * This represents a LLVM module, with added WhyR annotations.
     */
    class AnnotatedModule {
    protected:
        unique_ptr<Module> llvm;
        list<AnnotatedFunction*> functions;
        WhyRSettings* settings;
    public:
        AnnotatedModule(unique_ptr<Module>& llvm, WhyRSettings* settings = NULL);
        ~AnnotatedModule();
        
        /**
         * Constructing an AnnotatedModule does not annotate it.
         * Call this function so it can parse any attached functions.
         */
        void annotate();
        /**
         * Returns the list of AnnotatedFunction s in the module. Does not return NULL.
         * This list will be empty if you do not call annotate() first!
         * 
         * This object owns the resulting list, and all elements. It will free them on deletion.
         */
        list<AnnotatedFunction*>* getFunctions();
        /**
         * Call this function to find the AnnotatedFunction with a given name.
         * Returns the AnnotatedFunction in question, or NULL if no such function exists.
         * 
         * This object owns the resulting AnnotatedFunction. It will free it on deletion.
         */
        AnnotatedFunction* getFunction(const char* name);
        /**
         * Call this function to find the AnnotatedFunction corresponding to an Function.
         * It returns the AnnotatedFunction representing the Function, or NULL if it does not exist in this module.
         * 
         * This object owns the resulting AnnotatedFunction. It will free it on deletion.
         */
        AnnotatedFunction* getFunction(Function* func);
        /**
         * Returns the Module used to create this AnnotatedModule.
         * 
         * This object owns the resulting Module. It will free it on deletion.
         */
        Module* rawIR();
        /**
         * Returns the settings used to create this module.
         * 
         * The function that created this AnnotatedModule owns the resulting WhyRSettings.
         * It will only free it after this AnnotatedModule is deleted.
         */
        WhyRSettings* getSettings();
        
        /**
         * Retrieves a module from an input stream consisting of LLVM bitcode.
         * Returns NULL if something went wrong.
         * file_name is important only for debug information.
         * 
         * The caller owns the resulting AnnotatedModule. Free it when you are done.
         */
        static AnnotatedModule* moduleFromBitcode(istream& file, const char* file_name, WhyRSettings* settings = NULL);
        /**
         * Retrieves a module from an input stream consisting of LLVM IR.
         * Returns NULL if something went wrong.
         * file_name is important only for debug information.
         * 
         * The caller owns the resulting AnnotatedModule. Free it when you are done.
         */
        static AnnotatedModule* moduleFromIR(istream& file, const char* file_name, WhyRSettings* settings = NULL);
    };
}

#endif /* INCLUDE_WHYR_MODULE_HPP_ */
