/*
 * export.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: jrobbins
 */

#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>
#include <whyr/types.hpp>

#include <cmath>
#include <sstream>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    /*
     * TYPE DISCOVERY
     */
    
    void getTypeInfo(TypeInfo &info, Type* type) {
        if (type->isFloatingPointTy()) {
            info.floatTypes.insert(type);
        } else if (type->isIntegerTy()) {
            info.intTypes.insert(cast<IntegerType>(type));
        } else if (type->isPointerTy()) {
            info.ptrTypes.insert(cast<PointerType>(type));
            getTypeInfo(info, type->getPointerElementType());
        } else if (type->isArrayTy()) {
            info.arrayTypes.insert(cast<ArrayType>(type));
            getTypeInfo(info, type->getArrayElementType());
        } else if (type->isStructTy()) {
            info.structTypes.insert(cast<StructType>(type));
            for (unsigned i = 0; i < type->getStructNumElements(); i++) {
                getTypeInfo(info, type->getStructElementType(i));
            }
        } else if (type->isVectorTy()) {
            info.vectorTypes.insert(cast<VectorType>(type));
            getTypeInfo(info, type->getVectorElementType());
        } else if (type->isVoidTy() || type->isLabelTy()) {
            // do nothing
        } else {
            // give a warning
            if (info.module && info.module->getSettings()) {
                info.module->getSettings()->warnings.push_back(whyr_warning("found unknown type in input"));
            } else {
                // Can't warn. We have no output channel to do it in without crashing.
            }
        }
    }
    
    void getTypeInfo(TypeInfo &info, AnnotatedFunction* func, Instruction* inst) {
        // Set the module we're looking at
        info.module = func->getModule();
        
        // get info about the instruction's return type
        getTypeInfo(info, inst->getType());
        
        // get info about statepoints
        info.statepoints.insert(getWhy3StatepointBefore(func, inst));
        
        // if we're a call instruction, we need to add the called function to the function list
        if (isa<CallInst>(inst)) {
            CallInst* ii = cast<CallInst>(inst);
            
            if (ii->getCalledFunction()) {
                info.funcsCalled.insert(ii->getCalledFunction());
            } else {
                getTypeInfo(info, ii->getFunctionType());
            }
            
            // if this instruction has a return value, record it as a local
            if (!inst->getType()->isVoidTy()) {
                info.locals.insert(inst);
            }
            
            // Get the info for all of the operand's types
            // Different than below, because we already specially handled the function pointer operand
            for (unsigned i = 0; i < ii->getNumArgOperands(); i++) {
                Value* operand = ii->getArgOperand(i);
                if (isa<GlobalValue>(operand)) {
                    info.globalsUsed.insert(cast<GlobalValue>(operand));
                } else if (isa<BlockAddress>(operand)) {
                    BlockAddress* baddr = cast<BlockAddress>(operand);
                    if (baddr->getFunction() != func->rawIR()) info.funcsCalled.insert(baddr->getFunction());
                    info.usesBaddr = true;
                } else if (isa<Constant>(operand) || isa<BasicBlock>(operand) || isa<MetadataAsValue>(operand)) {
                    // ignore
                } else {
                    // Else, if it is a local variable...
                    info.locals.insert(operand);
                }
                getTypeInfo(info, operand->getType());
            }
            
            return;
        }
        
        // indirectbr instructions use BlockAddress
        if (isa<IndirectBrInst>(inst)) {
            info.usesBaddr = true;
        }
        
        // alloca instructions use the Alloc theory
        if (isa<AllocaInst>(inst)) {
            info.usesAlloc = true;
        }
        
        // if this instruction has a return value, record it as a local
        if (!inst->getType()->isVoidTy()) {
            info.locals.insert(inst);
        }
        
        // Get the info for all of the operands' types
        for (unsigned i = 0; i < inst->getNumOperands(); i++) {
            Value* operand = inst->getOperand(i);
            if (isa<GlobalValue>(operand)) {
                info.globalsUsed.insert(cast<GlobalValue>(operand));
            } else if (isa<BlockAddress>(operand)) {
                BlockAddress* baddr = cast<BlockAddress>(operand);
                if (baddr->getFunction() != func->rawIR()) info.funcsCalled.insert(baddr->getFunction());
                info.usesBaddr = true;
            } else if (isa<Constant>(operand) || isa<BasicBlock>(operand) || isa<MetadataAsValue>(operand)) {
                // ignore
            } else {
                // Else, if it is a local variable...
                info.locals.insert(operand);
            }
            getTypeInfo(info, operand->getType());
        }
        
        // note that we can't gather data from assert/assume clauses here- we do that in the AnnotatedFunction version of this function.
    }
    
    void getTypeInfo(TypeInfo &info, AnnotatedFunction* func) {
        // Set the module we're looking at
        info.module = func->getModule();
        
        // get info about the function's return type
        getTypeInfo(info, func->rawIR()->getReturnType());
        
        // Get the info for all of the arguments' types
        for (iplist<Argument>::iterator ii = func->rawIR()->getArgumentList().begin(); ii != func->rawIR()->getArgumentList().end(); ii++) {
            getTypeInfo(info, ii->getType());
            info.locals.insert(&*ii);
        }
        
        // Get the info for each instruction present in the function
        for (Function::iterator ii = func->rawIR()->begin(); ii != func->rawIR()->end(); ii++) {
            for (BasicBlock::iterator jj = ii->begin(); jj != ii->end(); jj++) {
                getTypeInfo(info, func, &*jj);
            }
        }
        
        // If we have a requires/ensures clause, it may need to import more types
        if (func->getRequiresClause()) {
            ostringstream discarded; // FIXME: this is not an efficient way to discard stream data. We need a custom ostream subclass.
            
            Why3Data data;
            data.module = func->getModule();
            data.source = new NodeSource(func);
            data.info = &info;
            
            func->getRequiresClause()->toWhy3(discarded, data);
        }
        
        if (func->getEnsuresClause()) {
            ostringstream discarded; // FIXME: this is not an efficient way to discard stream data. We need a custom ostream subclass.
            
            Why3Data data;
            data.module = func->getModule();
            data.source = new NodeSource(func);
            data.info = &info;
            
            func->getEnsuresClause()->toWhy3(discarded, data);
        }
        
        // For every instruction with annotations, gather info about thier assert/assume clauses
        for (list<AnnotatedInstruction*>::iterator ii = func->getAnnotatedInstructions()->begin(); ii != func->getAnnotatedInstructions()->end(); ii++) {
            ostringstream discarded;
            
            Why3Data data;
            data.module = func->getModule();
            data.source = new NodeSource(func);
            data.info = &info;
            
            if ((*ii)->getAssertClause()) {
                (*ii)->getAssertClause()->toWhy3(discarded, data);
            }
            
            if ((*ii)->getAssumeClause()) {
                (*ii)->getAssumeClause()->toWhy3(discarded, data);
            }
        }
    }
    
    void getTypeInfo(TypeInfo &info, AnnotatedModule* module) {
        // Set the module we're looking at
        info.module = module;
        
        // Gather info about all the functions in the module
        list<AnnotatedFunction*>* a = module->getFunctions();
        for (list<AnnotatedFunction*>::iterator ii = a->begin(); ii != a->end(); ii++) {
            getTypeInfo(info, *ii);
        }
        
        // Gather info about all the globals' types in the module
        for (iplist<GlobalVariable>::iterator ii = module->rawIR()->getGlobalList().begin(); ii != module->rawIR()->getGlobalList().end(); ii++) {
            getTypeInfo(info, ii->getType());
        }
    }
    
    void getTypeInfo(TypeInfo &info, TypeInfo &other) {
        info.module = other.module;
        info.intTypes.insert(other.intTypes.begin(), other.intTypes.end());
        info.ptrTypes.insert(other.ptrTypes.begin(), other.ptrTypes.end());
        info.funcsCalled.insert(other.funcsCalled.begin(), other.funcsCalled.end());
        info.globalsUsed.insert(other.globalsUsed.begin(), other.globalsUsed.end());
        info.usesAlloc = info.usesAlloc | other.usesAlloc;
    }
    
    /*
     * NAME MANGLER
     */
    
    static unordered_map<void*, string> temp_names;
    static int temp_name_counter = 0;
    string getTempName(void* p) {
        unordered_map<void*, string>::iterator ii = temp_names.find(p);
        if (ii == temp_names.end()) {
            string name = to_string(temp_name_counter++);
            temp_names[p] = name;
            return name;
        } else {
            return ii->second;
        }
    }
    
    string getWhy3SafeName(string name) {
        ostringstream out;
        bool onlyNumbers = true;
        for (unsigned i = 0; i < name.size(); i++) {
            char c = name[i];
            
            if (c < '0' || c > '9') {
                onlyNumbers = false;
            }
            
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
                out << c;
            } else {
                // if c is not in the set [a-zA-Z0-9_], mangle it
                out << '\'' << (unsigned)c;
            }
        }
        if (onlyNumbers) {
            // digit-only names could be confused with temp names. Add a ' on the end to make it more than digits.
            out << '\'';
        }
        return out.str();
    }
    
    string getWhy3TheoryName(Type* type) {
        if (type->isIntegerTy()) {
            return "I" + to_string(type->getIntegerBitWidth());
        } else if (type->isPointerTy()) {
            return getWhy3TheoryName(type->getPointerElementType()) + "P";
        } else if (type->isFloatTy()) {
            return "Float";
        } else if (type->isDoubleTy()) {
            return "Double";
        } else if (type->isArrayTy()) {
            return "Array" + getWhy3TheoryName(type->getArrayElementType()) + "_" + to_string(type->getArrayNumElements()); 
        } else if (type->isStructTy()) {
            if (cast<StructType>(type)->isLiteral()) {
                ostringstream name;
                name << "Struct";
                if (cast<StructType>(type)->isPacked()) {
                    name << "Packed";
                }
                for (unsigned i = 0; i < type->getStructNumElements(); i++) {
                    name << "_" << getWhy3TheoryName(type->getStructElementType(i));
                }
                return name.str();
            } else {
                return "Type_" + getWhy3SafeName(string(type->getStructName().data()));
            }
        } else if (type->isVectorTy()) {
            return "Vector" + getWhy3TheoryName(type->getVectorElementType()) + "_" + to_string(type->getVectorNumElements()); 
        } else {
            throw llvm_exception("Unknown type in LLVM input");
        }
    }
    
    string getWhy3TypeName(Type* type) {
        if (type->isIntegerTy()) {
            return "i" + to_string(type->getIntegerBitWidth());
        } else if (type->isPointerTy()) {
            return getWhy3TypeName(type->getPointerElementType()) + "p";
        } else if (type->isFloatTy()) {
            return "float";
        } else if (type->isDoubleTy()) {
            return "double";
        } else if (type->isArrayTy()) {
            return "a" + getWhy3TypeName(type->getArrayElementType()) + "_" + to_string(type->getArrayNumElements());
        } else if (type->isStructTy()) {
            if (cast<StructType>(type)->isLiteral()) {
                ostringstream name;
                name << "struct";
                if (cast<StructType>(type)->isPacked()) {
                    name << "_packed";
                }
                for (unsigned i = 0; i < type->getStructNumElements(); i++) {
                    name << "_" << getWhy3TypeName(type->getStructElementType(i));
                }
                return name.str();
            } else {
                return "type_" + getWhy3SafeName(string(type->getStructName().data()));
            }
        } else if (type->isVectorTy()) {
            return "v" + getWhy3TypeName(type->getVectorElementType()) + "_" + to_string(type->getVectorNumElements()); 
        } else {
            throw llvm_exception("Unknown type in LLVM input");
        }
    }
    
    string getWhy3FullName(Type* type) {
        return getWhy3TheoryName(type) + "." + getWhy3TypeName(type);
    }
    
    string getWhy3TheoryName(AnnotatedFunction* func) {
        return "Function_" + getWhy3SafeName(string(func->rawIR()->getName().data()));
    }
    
    string getWhy3VarName(Value* var) {
        string name;
        
        if (var->hasName()) {
            name = getWhy3SafeName(var->getName().data());
        } else {
            name = getTempName(var);
        }
        
        return string("val_") + name;
    }
    
    string getWhy3ArgName(string callee, Value* var) {
        return "arg_" + getWhy3SafeName(callee) + "_" + getWhy3SafeName(getWhy3VarName(var));
    }
    
    string getWhy3BlockName(AnnotatedFunction* func, BasicBlock* block) {
        if (block->hasName()) {
            return "b_" + getWhy3SafeName(string(block->getName().data()));
        }
        return "b_" + getTempName(block);
    }
    
    string getWhy3StatementName(AnnotatedFunction* func, Instruction* inst) {
        unsigned inst_no = 1;
        for (BasicBlock::iterator ii = inst->getParent()->begin(); ii != inst->getParent()->end(); ii++) {
            if (&*ii == inst) break;
            inst_no++;
        }
        return getWhy3BlockName(func, inst->getParent()) + "_" + inst->getOpcodeName() + "_" + to_string(inst_no);
    }
    
    string getWhy3LocalName(LogicLocal* local) {
        return "local_" + getWhy3SafeName(local->name);
    }
    
    string getWhy3StatepointBeforeBlock(AnnotatedFunction* func, BasicBlock* block) {
        return "state_before_" + getWhy3BlockName(func, block);
    }
    
    string getWhy3StatepointBefore(AnnotatedFunction* func, Instruction* inst) {
        BasicBlock* block = inst->getParent();
        
        string state = "entry_state";
        if (&func->rawIR()->getEntryBlock() != block) {
            state = getWhy3StatepointBeforeBlock(func, block);
        }
        
        for (BasicBlock::iterator ii = block->begin(); ii != block->end(); ii++) {
            if (&*ii == inst) {
                return state;
            }
            switch (ii->getOpcode()) {
                case Instruction::MemoryOps::Store:
                case Instruction::MemoryOps::Alloca:
                case Instruction::OtherOps::Call: {
                    state = "state_after_" + getWhy3BlockName(func, block) + "_" + ii->getOpcodeName() + "_" + getTempName(&*ii);
                    break;
                }
                default: {
                    // do nothing
                }
            }
        }
        
        throw whyr_exception("Internal error: in getWhy3StatepointBefore: inst not part of parent block!");
    }
    
    string getWhy3GlobalName(GlobalVariable* global) {
        return string("global_") + getWhy3SafeName(global->getName().data());
    }
    
    string getWhy3GlobalName(GlobalValue* global) {
        return string("global_") + getWhy3SafeName(global->getName().data());
    }
    
    string getWhy3StructFieldName(AnnotatedModule* module, StructType* type, unsigned index) {
        return getWhy3TypeName(type) + "_field_" + to_string(index);
    }
    
    // ABOUT CustomTruncate BELOW:
    // There is a bug in Alt-Ergo that causes all proofs to run forever when Why3's real.Truncate theory is imported.
    // We add a custom truncation theory that is more limited than Why3's. This reduces the impact of the bug, but does not eliminate it.
    // Things using CustomTruncate are unessecarily slow because of this bug.
    
    static const string why3_header =
R"((*
==============================================
This file was automatically generated by WhyR.
==============================================
*)

theory CustomTruncate
    use import int.Int
    use import real.Real
    use import real.FromInt
    function truncate real : int
    axiom Truncate_int : forall i:int. truncate (from_int i) = i
    axiom Truncate_down_pos: forall x:real. x >= 0.0 -> from_int (truncate x) <= x < from_int (Int.(+) (truncate x) 1)
    axiom Truncate_up_neg: forall x:real. x <= 0.0 -> from_int (Int.(-) (truncate x) 1) < x <= from_int (truncate x)
    axiom Real_of_truncate: forall x:real. x - 1.0 <= from_int (truncate x) <= x + 1.0
    axiom Truncate_monotonic: forall x y:real. x <= y -> Int.(<=) (truncate x) (truncate y)
    axiom Truncate_monotonic_int1: forall x:real, i:int. x <= from_int i -> Int.(<=) (truncate x) i
    axiom Truncate_monotonic_int2: forall x:real, i:int. from_int i <= x -> Int.(<=) i (truncate x)
end
)";

    void addImports(ostream &out, NodeSource* source, TypeInfo &info) {
        // This is a wrapper around the Why3Data version of this function.
        Why3Data data;
        data.module = info.module;
        data.source = source;
        data.info = &info;
        
        addImports(out, data);
    }
    
    void addImports(ostream &out, Why3Data &data) {
        // we need these for pretty much all arithmetic; so just include them by default
        out << "    use import int.Int" << endl;
        out << "    use import real.RealInfix" << endl;
        // for each field in TypeInfo, add the relevant asserts.
        for (unordered_set<IntegerType*>::iterator ii = data.info->intTypes.begin(); ii != data.info->intTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        for (unordered_set<PointerType*>::iterator ii = data.info->ptrTypes.begin(); ii != data.info->ptrTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        for (unordered_set<Type*>::iterator ii = data.info->floatTypes.begin(); ii != data.info->floatTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        for (unordered_set<ArrayType*>::iterator ii = data.info->arrayTypes.begin(); ii != data.info->arrayTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        for (unordered_set<StructType*>::iterator ii = data.info->structTypes.begin(); ii != data.info->structTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        for (unordered_set<VectorType*>::iterator ii = data.info->vectorTypes.begin(); ii != data.info->vectorTypes.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        if (data.info->usesAlloc) {
            out << "    use import Alloc" << endl;
        }
        if (data.info->usesBaddr) {
            out << "    use import BlockAddress" << endl;
        }
        // Why3Data also includes needed import information.
        for (unordered_set<string>::iterator ii = data.importsNeeded.begin(); ii != data.importsNeeded.end(); ii++) {
            out << "    use import " << *ii << endl;
        }
    }
    
    void addLLVMIntConstant(ostream &out, AnnotatedModule* module, Type* type, string number) {
        if (!module->getSettings() || module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_INT) {
            out << number;
        } else if (module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_BV) {
            out << "(" << getWhy3TheoryName(type) << ".of_int " << number << ")";
        }
    }
    
    void addLLVMFloatConstant(ostream &out, AnnotatedModule* module, Type* type, string number) {
        if (!module->getSettings() || module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_REAL) {
            out << number;
        } else if (module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_FP) {
            out << "(" << getWhy3TheoryName(type) << ".of_real Rounding.NearestTiesToEven " << number << ")";
        }
    }
    
    void addOperand(ostream &out, AnnotatedModule* module, Value* operand, AnnotatedFunction* func) {
        // TODO: add handlers for different subclasses of Constant, etc.
        if (isa<GlobalValue>(operand)) {
            out << getWhy3GlobalName(cast<GlobalValue>(operand));
        } else if (isa<ConstantInt>(operand)) {
            ConstantInt* cc = cast<ConstantInt>(operand);
            string s = cc->getValue().toString(10, false);
            if (s[0] == '-') {
                s = "(" + s + ")";
            }
            addLLVMIntConstant(out, module, cc->getType(), s);
        } else if (isa<ConstantFP>(operand)) {
            ConstantFP* cc = cast<ConstantFP>(operand);
            SmallVector<char, 0> vec;
            cc->getValueAPF().toString(vec);
            
            if (find(vec.begin(), vec.end(), '.') == vec.end()) {
                vec.push_back('.');
                vec.push_back('0');
            }
            
            if (vec.front() == '-') {
                SmallVector<char, 0> vec2;
                vec2.push_back('(');
                vec2.push_back('-');
                vec2.push_back('.');
                vec2.push_back(' ');
                SmallVector<char, 0>::iterator begin = vec.begin();
                begin++;
                for (SmallVector<char, 0>::iterator ii = begin; ii != vec.end(); ii++) {
                    vec2.push_back(*ii);
                }
                vec2.push_back(')');
                vec = vec2;
            }
            
            vec.push_back('\0');
            addLLVMFloatConstant(out, module, cc->getType(), string(vec.data()));
        } else if (isa<ConstantArray>(operand)) {
            ConstantArray* cc = cast<ConstantArray>(operand);
            out << getWhy3TheoryName(operand->getType()) << ".any_array";
            for (unsigned i = 0; i < cc->getNumOperands(); i++) {
                out << "[" << i << " <- ";
                addOperand(out, module, cc->getOperand(i), func);
                out << "]";
            }
        } else if (isa<ConstantDataArray>(operand)) {
            // data arrays are the same as arrays, LLVM just stores it in a more compact format
            ConstantDataArray* cc = cast<ConstantDataArray>(operand);
            out << getWhy3TheoryName(operand->getType()) << ".any_array";
            for (unsigned i = 0; i < cc->getNumElements(); i++) {
                out << "[" << i << " <- ";
                addOperand(out, module, cc->getElementAsConstant(i), func);
                out << "]";
            }
        } else if (isa<ConstantStruct>(operand)) {
            ConstantStruct* cc = cast<ConstantStruct>(operand);
            out << "{ ";
            for (unsigned i = 0; i < operand->getType()->getStructNumElements(); i++) {
                out << getWhy3TheoryName(operand->getType()) << "." << getWhy3StructFieldName(module, cc->getType(), i) << " = ";
                addOperand(out, module, cc->getAggregateElement(i));
                out << "; ";
            }
            out << "}";
        } else if (isa<ConstantAggregateZero>(operand)) {
            if (operand->getType()->isArrayTy()) {
                Constant* value = cast<ConstantAggregateZero>(operand)->getSequentialElement();
                
                out << getWhy3TheoryName(operand->getType()) << ".any_array";
                for (unsigned i = 0; i < operand->getType()->getArrayNumElements(); i++) {
                    out << "[" << i << " <- ";
                    addOperand(out, module, value, func);
                    out << "]";
                }
            } else if (operand->getType()->isStructTy()) {
                ConstantAggregateZero* cc = cast<ConstantAggregateZero>(operand);
                out << "{ ";
                for (unsigned i = 0; i < operand->getType()->getStructNumElements(); i++) {
                    out << getWhy3TheoryName(operand->getType()) << "." << getWhy3StructFieldName(module, cast<StructType>(operand->getType()), i) << " = ";
                    addOperand(out, module, cc->getStructElement(i));
                    out << "; ";
                }
                out << "}";
            } else if (operand->getType()->isVectorTy()) {
                Constant* value = cast<ConstantAggregateZero>(operand)->getSequentialElement();
                
                out << getWhy3TheoryName(operand->getType()) << ".any_vector";
                for (unsigned i = 0; i < operand->getType()->getVectorNumElements(); i++) {
                    out << "[" << i << " <- ";
                    addOperand(out, module, value, func);
                    out << "]";
                }
            } else {
                throw type_exception("Instantiation of aggregate zero of type '" + LogicTypeLLVM(operand->getType()).toString() + "' currently unsupported");
            }
        } else if (isa<ConstantPointerNull>(operand)) {
            out << getWhy3TheoryName(operand->getType()) << ".null";
        } else if (isa<BlockAddress>(operand)) {
            BlockAddress* baddr = cast<BlockAddress>(operand);
            out << "(store_baddr ";
            
            AnnotatedFunction* baddrFunc = module->getFunction(baddr->getFunction());
            if (baddrFunc == func) {
                out << getWhy3BlockName(baddrFunc, baddr->getBasicBlock());
            } else {
                out << "true";
            }
            
            out << ")";
        } else if (isa<ConstantVector>(operand)) {
            ConstantVector* cc = cast<ConstantVector>(operand);
            out << getWhy3TheoryName(operand->getType()) << ".any_vector";
            for (unsigned i = 0; i < cc->getNumOperands(); i++) {
                out << "[" << i << " <- ";
                addOperand(out, module, cc->getOperand(i), func);
                out << "]";
            }
        } else if (isa<ConstantDataVector>(operand)) {
            // data vectors are just another vector constant
            ConstantDataVector* cc = cast<ConstantDataVector>(operand);
            out << getWhy3TheoryName(operand->getType()) << ".any_vector";
            for (unsigned i = 0; i < cc->getNumElements(); i++) {
                out << "[" << i << " <- ";
                addOperand(out, module, cc->getElementAsConstant(i), func);
                out << "]";
            }
        } else {
            // Else, if it is a local variable...
            out << getWhy3VarName(operand);
        }
    }
    
    void addWhy3PhiImplications(ostream &out, AnnotatedFunction* func, BasicBlock* block, BasicBlock* succ) {
        for (BasicBlock::iterator ii = succ->begin(); ii != succ->end(); ii++) {
            if (isa<PHINode>(&*ii)) {
                PHINode* phi = cast<PHINode>(&*ii);
                
                out << " -> ";
                addOperand(out, func->getModule(), phi, func);
                out << " = ";
                addOperand(out, func->getModule(), phi->getIncomingValueForBlock(block), func);
            }
        }
    }
    
    void addInstruction(ostream &out, AnnotatedFunction* func, Instruction* inst, LogicExpression* goalExpr, Instruction* goalInst) {
        bool combineGoals = (func->getModule()->getSettings() && func->getModule()->getSettings()->combineGoals);
        
        switch (inst->getOpcode()) {
            case Instruction::BinaryOps::FAdd:
            case Instruction::BinaryOps::FSub:
            case Instruction::BinaryOps::FMul:
            case Instruction::BinaryOps::FDiv:
            case Instruction::BinaryOps::FRem:
            case Instruction::CastOps::FPTrunc:
            case Instruction::CastOps::FPExt: {
                out << "    use import floating_point.Rounding" << endl;
                break;
            }
            case Instruction::CastOps::FPToUI:
            case Instruction::CastOps::FPToSI: {
                out << "    use import floating_point.Rounding" << endl;
                out << "    use import CustomTruncate" << endl;
                break;
            }
            case Instruction::CastOps::UIToFP:
            case Instruction::CastOps::SIToFP: {
                out << "    use import floating_point.Rounding" << endl;
                out << "    use import real.FromInt" << endl;
                break;
            }
            case Instruction::CastOps::PtrToInt:
            case Instruction::CastOps::IntToPtr: {
                unsigned ptrBits = func->getModule()->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
                Type* ptrIntType = IntegerType::get(func->getModule()->rawIR()->getContext(), ptrBits);
                if (inst->getType()->isVectorTy()) {
                    ptrIntType = VectorType::get(ptrIntType, inst->getType()->getVectorNumElements());
                }
                
                out << "    use import " << getWhy3TheoryName(ptrIntType) << endl;
                break;
            }
            case Instruction::MemoryOps::GetElementPtr:
            case Instruction::CastOps::BitCast: {
                out << "    use import Pointer" << endl;
                break;
            }
            case Instruction::OtherOps::Call: {
                CallInst* callInst = cast<CallInst>(inst);
                Function* calledFuncRaw = callInst->getCalledFunction();
                if (!calledFuncRaw) {
                    // give a warning
                    if (func->getModule()->getSettings()) {
                        func->getModule()->getSettings()->warnings.push_back(whyr_warning("indirect calls currently unsupported; call instruction ignored", NULL, new NodeSource(func, inst)));
                    } else {
                        // Can't warn. We have no output channel to do it in without crashing.
                    }
                    
                    // make instruction a no-op
                    out << "    predicate " << getWhy3StatementName(func, inst) << " = true" << endl;
                    
                    return;
                }
                
                AnnotatedFunction* calledFunc = func->getModule()->getFunction(calledFuncRaw);
                if (!calledFunc) {
                    // should never get here, but who knows
                    throw whyr_exception(("internal error: function " + string(calledFuncRaw->getName().data()) + " not found in module"), NULL, new NodeSource(func, inst));
                }
                string calleeTheoryName = getWhy3TheoryName(calledFunc) + "_call_" + getTempName(inst);
                
                unsigned i;
                i = 0;
                for (Function::ArgumentListType::iterator ii = calledFuncRaw->getArgumentList().begin(); ii != calledFuncRaw->getArgumentList().end(); ii++) {
                    out << "    constant " << getWhy3ArgName(calleeTheoryName, &*ii) << " : " << getWhy3FullName(ii->getType()) << " = ";
                    addOperand(out, func->getModule(), callInst->getArgOperand(i), func);
                    out << endl;
                    i++;
                }
                out << "    namespace import " << calleeTheoryName << endl;
                out << "        clone import " << getWhy3TheoryName(calledFunc) << " as F with " << endl;
                if (!calledFuncRaw->getArgumentList().empty()) {
                    i = 0;
                    for (Function::ArgumentListType::iterator ii = calledFuncRaw->getArgumentList().begin(); ii != calledFuncRaw->getArgumentList().end(); ii++) {
                        if (i) out << "," << endl;
                        out << "            constant " << getWhy3VarName(&*ii) << " = " << getWhy3ArgName(calleeTheoryName, &*ii);
                        i++;
                    }
                    out << "," << endl << "            constant entry_state = " << getWhy3StatepointBefore(func, inst) << endl;
                } else {
                    out << "            constant entry_state = " << getWhy3StatepointBefore(func, inst) << endl;
                }
                out << "    end" << endl;
                
                break;
            }
            case Instruction::CastOps::Trunc: {
                out << "    use import int.ComputerDivision" << endl;
                break;
            }
            default: {
                // do nothing
            }
        }
        
        out << "    predicate " << getWhy3StatementName(func, inst) << " = (";
        switch (inst->getOpcode()) {
            case Instruction::TermOps::Br: {
                BranchInst* brInst = cast<BranchInst>(inst);
                if (brInst->isConditional()) {
                    out << "if ";
                    addOperand(out, func->getModule(), brInst->getCondition(), func);
                    out << " = ";
                    addLLVMIntConstant(out, func->getModule(), brInst->getCondition()->getType(), "1");
                    out << " then (" << getWhy3StatepointBeforeBlock(func, brInst->getSuccessor(0)) << " = " << getWhy3StatepointBefore(func, inst);
                    addWhy3PhiImplications(out, func, inst->getParent(), brInst->getSuccessor(0));
                    out << " -> " << getWhy3BlockName(func, brInst->getSuccessor(0)) << ")";
                    out << " else (" << getWhy3StatepointBeforeBlock(func, brInst->getSuccessor(1)) << " = " << getWhy3StatepointBefore(func, inst);
                    addWhy3PhiImplications(out, func, inst->getParent(), brInst->getSuccessor(1));
                    out << " -> " << getWhy3BlockName(func, brInst->getSuccessor(1)) << ")";
                } else {
                    out << getWhy3StatepointBeforeBlock(func, brInst->getSuccessor(0)) << " = " << getWhy3StatepointBefore(func, inst);
                    addWhy3PhiImplications(out, func, inst->getParent(), brInst->getSuccessor(0));
                    out << " -> " << getWhy3BlockName(func, brInst->getSuccessor(0));
                }
                break;
            }
            case Instruction::TermOps::Ret: {
                if (func->rawIR()->getReturnType()->isVoidTy()) {
                    out << "exit_state = " << getWhy3StatepointBefore(func, inst) << " -> function_ensures";
                } else {
                    out << "ret_val = ";
                    addOperand(out, func->getModule(), inst->getOperand(0), func);
                    out << " -> exit_state = " << getWhy3StatepointBefore(func, inst) << " -> function_ensures";
                }
                
                if (!combineGoals && goalExpr != func->getEnsuresClause()) {
                    out << " -> true";
                }
                break;
            }
            case Instruction::TermOps::Switch: {
                SwitchInst* switchInst = cast<SwitchInst>(inst);
                
                if (switchInst->getNumCases() == 0) {
                    // special case where this acts like a unconditiuonal branch
                    out << getWhy3StatepointBeforeBlock(func, switchInst->getDefaultDest()) << " = " << getWhy3StatepointBefore(func, inst);
                    addWhy3PhiImplications(out, func, inst->getParent(), switchInst->getDefaultDest());
                    out << " -> " << getWhy3BlockName(func, switchInst->getDefaultDest());
                } else {
                    out << endl << "        ";
                    // add cases
                    for (SwitchInst::CaseIt ii = switchInst->cases().begin(); ii != switchInst->cases().end(); ii++) {
                        ConstantInt* value = ii.getCaseValue();
                        BasicBlock* succ = ii.getCaseSuccessor();
                        
                        out << "if ";
                        addOperand(out, func->getModule(), switchInst->getCondition(), func);
                        out << " = ";
                        addOperand(out, func->getModule(), value, func);
                        out << " then ";
                        out << getWhy3StatepointBeforeBlock(func, succ) << " = " << getWhy3StatepointBefore(func, inst);
                        addWhy3PhiImplications(out, func, inst->getParent(), succ);
                        out << " -> " << getWhy3BlockName(func, succ) << endl << "        else ";
                    }
                    // add default case
                    out << getWhy3StatepointBeforeBlock(func, switchInst->getDefaultDest()) << " = " << getWhy3StatepointBefore(func, inst);
                    addWhy3PhiImplications(out, func, inst->getParent(), switchInst->getDefaultDest());
                    out << " -> " << getWhy3BlockName(func, switchInst->getDefaultDest()) << endl;
                    // done
                    out << "    ";
                }
                break;
            }
            case Instruction::TermOps::IndirectBr: {
                IndirectBrInst* brInst = cast<IndirectBrInst>(inst);
                out << "true";
                for (unsigned i = 0; i < brInst->getNumDestinations(); i++) {
                    addWhy3PhiImplications(out, func, inst->getParent(), brInst->getDestination(i));
                }
                out << " -> (load_baddr ";
                addOperand(out, func->getModule(), brInst->getOperand(0), func);
                out << ")";
                break;
            }
            case Instruction::BinaryOps::Add:
            case Instruction::BinaryOps::Mul:
            case Instruction::BinaryOps::Sub:
            case Instruction::BinaryOps::SDiv:
            case Instruction::BinaryOps::SRem:
            case Instruction::BinaryOps::UDiv:
            case Instruction::BinaryOps::URem: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << "." << inst->getOpcodeName() << " ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << ")";
                break;
            }
            case Instruction::BinaryOps::FAdd:
            case Instruction::BinaryOps::FSub:
            case Instruction::BinaryOps::FMul:
            case Instruction::BinaryOps::FDiv:
            case Instruction::BinaryOps::FRem: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << "." << inst->getOpcodeName() << " Rounding.NearestTiesToEven ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << ")";
                break;
            }
            case Instruction::BinaryOps::And:
            case Instruction::BinaryOps::Or:
            case Instruction::BinaryOps::Xor: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".bw_" << inst->getOpcodeName() << " ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << ")";
                break;
            }
            case Instruction::BinaryOps::Shl: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".lsl ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " (to_uint ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << "))";
                break;
            }
            case Instruction::BinaryOps::LShr: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".lsr ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " (to_uint ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << "))";
                break;
            }
            case Instruction::BinaryOps::AShr: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".asr ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " (to_uint ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << "))";
                break;
            }
            case Instruction::CastOps::Trunc: {
                addOperand(out, func->getModule(), inst, func);
                if (inst->getType()->isVectorTy()) {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int " << getWhy3TheoryName(inst->getType()) << ".any_int_vector";
                    for (unsigned i = 0; i < inst->getType()->getVectorNumElements(); i++) {
                        out << "[" << i << " <- (mod (" << getWhy3TheoryName(inst->getOperand(0)->getType()->getVectorElementType()) << ".to_uint ";
                        addOperand(out, func->getModule(), inst->getOperand(0), func);
                        out << "[" << i << "]) 0b1";
                        for (unsigned j = 0; j < inst->getType()->getVectorElementType()->getIntegerBitWidth(); j++) {
                            out << "0";
                        }
                        out << ")]";
                    }
                    out << ")";
                } else {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int (mod (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_uint ";
                    addOperand(out, func->getModule(), inst->getOperand(0), func);
                    out << ") 0b1";
                    for (unsigned i = 0; i < inst->getType()->getIntegerBitWidth(); i++) {
                        out << "0";
                    }
                    out << "))";
                }
                break;
            }
            case Instruction::CastOps::ZExt: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_uint ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << "))";
                break;
            }
            case Instruction::CastOps::SExt: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_int ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << "))";
                break;
            }
            case Instruction::CastOps::FPTrunc:
            case Instruction::CastOps::FPExt: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_real Rounding.NearestTiesToEven (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_real Rounding.NearestTiesToEven ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << "))";
                break;
            }
            case Instruction::CastOps::FPToUI:
            case Instruction::CastOps::FPToSI: {
                addOperand(out, func->getModule(), inst, func);
                if (inst->getType()->isVectorTy()) {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int " << getWhy3TheoryName(inst->getType()) << ".any_int_vector";
                    for (unsigned i = 0; i < inst->getType()->getVectorNumElements(); i++) {
                        out << "[" << i << " <- (truncate (" << getWhy3TheoryName(inst->getOperand(0)->getType()->getVectorElementType()) << ".to_real Rounding.NearestTiesToEven ";
                        addOperand(out, func->getModule(), inst->getOperand(0), func);
                        out << "[" << i << "]))]";
                    }
                    out << ")";
                } else {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_int (truncate (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_real Rounding.NearestTiesToEven ";
                    addOperand(out, func->getModule(), inst->getOperand(0), func);
                    out << ")))";
                }
                break;
            }
            case Instruction::CastOps::UIToFP: {
                addOperand(out, func->getModule(), inst, func);
                if (inst->getType()->isVectorTy()) {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_real Rounding.NearestTiesToEven " << getWhy3TheoryName(inst->getType()) << ".any_real_vector";
                    for (unsigned i = 0; i < inst->getType()->getVectorNumElements(); i++) {
                        out << "[" << i << " <- (from_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()->getVectorElementType()) << ".to_uint ";
                        addOperand(out, func->getModule(), inst->getOperand(0), func);
                        out << "[" << i << "]))]";
                    }
                    out << ")";
                } else {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_real Rounding.NearestTiesToEven (from_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_uint ";
                    addOperand(out, func->getModule(), inst->getOperand(0), func);
                    out << ")))";
                }
                break;
            }
            case Instruction::CastOps::SIToFP: {
                addOperand(out, func->getModule(), inst, func);
                if (inst->getType()->isVectorTy()) {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_real Rounding.NearestTiesToEven " << getWhy3TheoryName(inst->getType()) << ".any_real_vector";
                    for (unsigned i = 0; i < inst->getType()->getVectorNumElements(); i++) {
                        out << "[" << i << " <- (from_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()->getVectorElementType()) << ".to_int ";
                        addOperand(out, func->getModule(), inst->getOperand(0), func);
                        out << "[" << i << "]))]";
                    }
                    out << ")";
                } else {
                    out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_real Rounding.NearestTiesToEven (from_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_int ";
                    addOperand(out, func->getModule(), inst->getOperand(0), func);
                    out << ")))";
                }
                break;
            }
            case Instruction::CastOps::PtrToInt: {
                unsigned ptrBits = func->getModule()->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
                
                addOperand(out, func->getModule(), inst, func);
                out << " = ";
                
                if (inst->getType()->getIntegerBitWidth() != ptrBits) {
                    Type* ptrIntType = IntegerType::get(func->getModule()->rawIR()->getContext(), ptrBits);
                    if (inst->getType()->isVectorTy()) {
                        ptrIntType = VectorType::get(ptrIntType, inst->getType()->getVectorNumElements());
                    }
                    
                    out << "(" << getWhy3TheoryName(inst->getType()) << ".of_int (" << getWhy3TheoryName(ptrIntType) << ".to_uint ";
                }
                
                out << "(" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_ptrint ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << ")";
                
                if (inst->getType()->getIntegerBitWidth() != ptrBits) {
                    out << "))";
                }
                
                break;
            }
            case Instruction::CastOps::IntToPtr: {
                unsigned ptrBits = func->getModule()->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
                
                addOperand(out, func->getModule(), inst, func);
                out << " = (" << getWhy3TheoryName(inst->getType()) << ".of_ptrint ";
                
                if (inst->getOperand(0)->getType()->getIntegerBitWidth() != ptrBits) {
                    Type* ptrIntType = IntegerType::get(func->getModule()->rawIR()->getContext(), ptrBits);
                    if (inst->getType()->isVectorTy()) {
                        ptrIntType = VectorType::get(ptrIntType, inst->getType()->getVectorNumElements());
                    }
                    
                    out << "(" << getWhy3TheoryName(ptrIntType) << ".of_int (" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".to_uint ";
                }
                
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                
                if (inst->getOperand(0)->getType()->getIntegerBitWidth() != ptrBits) {
                    out << "))";
                }
                
                out << ")";
                break;
            }
            case Instruction::CastOps::BitCast: {
                addOperand(out, func->getModule(), inst, func);
                out << " = (cast ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << ")";
                break;
            }
            case Instruction::MemoryOps::Store: {
                out << getWhy3StatepointBefore(func, inst->getNextNode()) << " = (" << getWhy3TheoryName(inst->getOperand(1)->getType()) << ".store " << getWhy3StatepointBefore(func, inst) << " ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out << " ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << ")";
                break;
            }
            case Instruction::MemoryOps::Load: {
                addOperand(out, func->getModule(), inst, func);
                out << " = " << "(" << getWhy3TheoryName(inst->getOperand(0)->getType()) << ".load " << getWhy3StatepointBefore(func, inst) << " ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << ")";
                break;
            }
            case Instruction::MemoryOps::Alloca: {
                AllocaInst* allocaInst = cast<AllocaInst>(inst);
                out << "(" << getWhy3StatepointBefore(func, inst->getNextNode()) << ", ";
                addOperand(out, func->getModule(), inst, func);
                out << ") = (alloc " << getWhy3StatepointBefore(func, inst) << " (" << getWhy3TheoryName(inst->getType()) << ".elem_size";
                if (allocaInst->isArrayAllocation()) {
                    out << " * (" << getWhy3TheoryName(allocaInst->getArraySize()->getType()) << ".to_uint ";
                    addOperand(out, func->getModule(), allocaInst->getArraySize(), func);
                    out << ")";
                }
                out << "))";
                
                break;
            }
            case Instruction::MemoryOps::GetElementPtr: {
                GetElementPtrInst* gepInst = cast<GetElementPtrInst>(inst);
                addOperand(out, func->getModule(), inst, func);
                if (inst->getType()->isVectorTy()) {
                    out << " = " << getWhy3TheoryName(inst->getType()) << ".any_vector";
                    for (unsigned i = 0; i < inst->getType()->getVectorNumElements(); i++) {
                        out << "[" << i << " <- (cast (" << getWhy3TheoryName(gepInst->getPointerOperandType()->isVectorTy() ? gepInst->getPointerOperandType()->getVectorElementType() : gepInst->getPointerOperandType()) << ".offset_pointer ";
                        addOperand(out, func->getModule(), gepInst->getPointerOperand(), func);
                        if (gepInst->getPointerOperandType()->isVectorTy()) out << "[" << i << "]";
                        out << " (";
                        Type* currentType = gepInst->getPointerOperandType()->isVectorTy() ? gepInst->getPointerOperandType()->getVectorElementType() : gepInst->getPointerOperandType();
                        if (gepInst->getNumIndices() == 0) {
                            out << "0";
                        }
                        for (GetElementPtrInst::op_iterator ii = gepInst->idx_begin(); ii != gepInst->idx_end(); ii++) {
                            if (ii != gepInst->idx_begin()) {
                                out << " + ";
                            }
                            if (currentType->isPointerTy()) {
                                out << "(" << getWhy3TheoryName(currentType) << ".size * (" << getWhy3TheoryName(ii->get()->getType()->isVectorTy() ? ii->get()->getType()->getVectorElementType() : ii->get()->getType()) << ".to_int ";
                                addOperand(out, func->getModule(), ii->get(), func);
                                if (ii->get()->getType()->isVectorTy()) out << "[" << i << "]";
                                out << "))";
                                currentType = currentType->getPointerElementType();
                            } else if (currentType->isArrayTy()) {
                                out << "(" << getWhy3TheoryName(currentType->getArrayElementType()) << ".size * (" << getWhy3TheoryName(ii->get()->getType()->isVectorTy() ? ii->get()->getType()->getVectorElementType() : ii->get()->getType()) << ".to_int ";
                                addOperand(out, func->getModule(), ii->get(), func);
                                if (ii->get()->getType()->isVectorTy()) out << "[" << i << "]";
                                out << "))";
                                currentType = currentType->getArrayElementType();
                            } else if (currentType->isStructTy()) {
                                unsigned index = cast<ConstantDataVector>(ii->get())->getElementAsInteger(i);
                                unsigned offset = func->getModule()->rawIR()->getDataLayout().getStructLayout(cast<StructType>(currentType))->getElementOffsetInBits(index);
                                out << offset;
                                currentType = currentType->getStructElementType(index);
                            } else if (currentType->isVectorTy()) {
                                out << "(" << getWhy3TheoryName(currentType->getVectorElementType()) << ".size * (" << getWhy3TheoryName(ii->get()->getType()->isVectorTy() ? ii->get()->getType()->getVectorElementType() : ii->get()->getType()) << ".to_int ";
                                addOperand(out, func->getModule(), ii->get(), func);
                                if (ii->get()->getType()->isVectorTy()) out << "[" << i << "]";
                                out << "))";
                                currentType = currentType->getVectorElementType();
                            } else {
                                throw whyr_exception("Internal error: Unknown index type to GEP instruction: " + LogicTypeLLVM(currentType).toString(), NULL, new NodeSource(func, inst));
                            }
                        }
                        out << ")))]";
                    }
                } else {
                    out << " = (cast (" << getWhy3TheoryName(gepInst->getPointerOperand()->getType()) << ".offset_pointer ";
                    addOperand(out, func->getModule(), gepInst->getPointerOperand(), func);
                    out << " (";
                    Type* currentType = gepInst->getPointerOperandType();
                    if (gepInst->getNumIndices() == 0) {
                        out << "0";
                    }
                    for (GetElementPtrInst::op_iterator ii = gepInst->idx_begin(); ii != gepInst->idx_end(); ii++) {
                        if (ii != gepInst->idx_begin()) {
                            out << " + ";
                        }
                        if (currentType->isPointerTy()) {
                            out << "(" << getWhy3TheoryName(currentType) << ".size * (" << getWhy3TheoryName(ii->get()->getType()) << ".to_int ";
                            addOperand(out, func->getModule(), ii->get(), func);
                            out << "))";
                            currentType = currentType->getPointerElementType();
                        } else if (currentType->isArrayTy()) {
                            out << "(" << getWhy3TheoryName(currentType->getArrayElementType()) << ".size * (" << getWhy3TheoryName(ii->get()->getType()) << ".to_int ";
                            addOperand(out, func->getModule(), ii->get(), func);
                            out << "))";
                            currentType = currentType->getArrayElementType();
                        } else if (currentType->isStructTy()) {
                            unsigned index = cast<ConstantInt>(ii->get())->getLimitedValue();
                            unsigned offset = func->getModule()->rawIR()->getDataLayout().getStructLayout(cast<StructType>(currentType))->getElementOffsetInBits(index);
                            out << offset;
                            currentType = currentType->getStructElementType(index);
                        } else if (currentType->isVectorTy()) {
                            out << "(" << getWhy3TheoryName(currentType->getVectorElementType()) << ".size * (" << getWhy3TheoryName(ii->get()->getType()) << ".to_int ";
                            addOperand(out, func->getModule(), ii->get(), func);
                            out << "))";
                            currentType = currentType->getVectorElementType();
                        } else {
                            throw whyr_exception("Internal error: Unknown index type to GEP instruction", NULL, new NodeSource(func, inst));
                        }
                    }
                    out << ")))";
                }
                break;
            }
            case Instruction::OtherOps::ExtractValue: {
                ExtractValueInst* exInst = cast<ExtractValueInst>(inst);
                
                addOperand(out, func->getModule(), inst, func);
                out << " = ";
                addOperand(out, func->getModule(), exInst->getAggregateOperand(), func);
                Type* operandType = exInst->getAggregateOperand()->getType();
                for (ExtractValueInst::idx_iterator ii = exInst->idx_begin(); ii != exInst->idx_end(); ii++) {
                    if (operandType->isArrayTy()) {
                        out << "[" << *ii << "]";
                        operandType = operandType->getArrayElementType();
                    } else if (operandType->isStructTy()) {
                        out << "." << getWhy3StructFieldName(func->getModule(), cast<StructType>(operandType), *ii);
                        operandType = operandType->getStructElementType(*ii);
                    }
                }
                break;
            }
            case Instruction::OtherOps::InsertValue: {
                InsertValueInst* insInst = cast<InsertValueInst>(inst);
                
                addOperand(out, func->getModule(), inst, func);
                out << " = ";
                
                int i = 0; Type* operandType = insInst->getAggregateOperand()->getType();
                for (InsertValueInst::idx_iterator ii = insInst->idx_begin(); ii != insInst->idx_end(); ii++) {
                    if (operandType->isStructTy()) {
                        out << "{";
                    }
                    addOperand(out, func->getModule(), insInst->getAggregateOperand(), func);
                    
                    Type* operandType2 = insInst->getAggregateOperand()->getType();
                    for (int j = 0; j < i; j++) {
                        if (operandType2->isArrayTy()) {
                            out << "[" << *(insInst->idx_begin()+j) << "]";
                            operandType2 = operandType2->getArrayElementType();
                        } else if (operandType2->isStructTy()) {
                            out << "." << getWhy3StructFieldName(func->getModule(), cast<StructType>(operandType2), *(insInst->idx_begin()+j));
                            operandType2 = operandType2->getStructElementType(*(insInst->idx_begin()+j));
                        }
                    }
                    
                    if (operandType->isArrayTy()) {
                        out << "[" << *ii << " <- ";
                        operandType = operandType->getArrayElementType();
                    } else if (operandType->isStructTy()) {
                        out << " with " << getWhy3TheoryName(operandType) << "." << getWhy3StructFieldName(func->getModule(), cast<StructType>(operandType), *ii) << " = ";
                        operandType = operandType->getStructElementType(*ii);
                    }
                    
                    i++;
                }
                
                Type* operandType3 = insInst->getAggregateOperand()->getType();
                addOperand(out, func->getModule(), insInst->getInsertedValueOperand(), func);
                string ending;
                for (int j = 0; j < i; j++) {
                    if (operandType3->isArrayTy()) {
                        ending = "]" + ending;
                        operandType3 = operandType3->getArrayElementType();
                    } else if (operandType3->isStructTy()) {
                        ending = ";}" + ending;
                        operandType3 = operandType3->getStructElementType(*(insInst->idx_begin()+j));
                    }
                }
                out << ending;
                break;
            }
            case Instruction::OtherOps::ICmp: {
                ICmpInst* icmpInst = cast<ICmpInst>(inst);
                
                addOperand(out, func->getModule(), inst, func);
                out << " = if (";
                string opStr;
                if (icmpInst->getPredicate() == llvm::ICmpInst::ICMP_EQ || icmpInst->getPredicate() == llvm::ICmpInst::ICMP_NE) {
                    switch (icmpInst->getPredicate()) {
                        case llvm::ICmpInst::ICMP_EQ: {
                            opStr = " = ";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_NE: {
                            opStr = " <> ";
                            break;
                        }
                        default: {
                            throw whyr_exception("Internal error: Undefined predicate value for icmp", NULL, new NodeSource(func, inst));
                        }
                    }
                    
                    addOperand(out, func->getModule(), icmpInst->getOperand(0), func);
                    out << opStr;
                    addOperand(out, func->getModule(), icmpInst->getOperand(1), func);
                } else {
                    switch (icmpInst->getPredicate()) {
                        case llvm::ICmpInst::ICMP_SGT: {
                            opStr = "sgt";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_SGE: {
                            opStr = "sge";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_SLT: {
                            opStr = "slt";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_SLE: {
                            opStr = "sle";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_UGT: {
                            opStr = "ugt";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_UGE: {
                            opStr = "uge";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_ULT: {
                            opStr = "ult";
                            break;
                        }
                        case llvm::ICmpInst::ICMP_ULE: {
                            opStr = "ule";
                            break;
                        }
                        default: {
                            throw whyr_exception("Internal error: Undefined predicate value for icmp", NULL, new NodeSource(func, inst));
                        }
                    }
                    
                    out << getWhy3TheoryName(icmpInst->getOperand(0)->getType()) << "." << opStr << " ";
                    addOperand(out, func->getModule(), icmpInst->getOperand(0), func);
                    out << " ";
                    addOperand(out, func->getModule(), icmpInst->getOperand(1), func);
                }
                out << ") then ";
                addLLVMIntConstant(out, func->getModule(), icmpInst->getType(), "1");
                out << " else ";
                addLLVMIntConstant(out, func->getModule(), icmpInst->getType(), "0");
                break;
            }
            case Instruction::OtherOps::FCmp: {
                FCmpInst* fcmpInst = cast<FCmpInst>(inst);
                
                addOperand(out, func->getModule(), inst, func);
                if (fcmpInst->getPredicate() == FCmpInst::FCMP_FALSE) {
                    out << " = ";
                    addLLVMIntConstant(out, func->getModule(), fcmpInst->getType(), "0");
                } else if (fcmpInst->getPredicate() == FCmpInst::FCMP_TRUE) {
                    out << " = ";
                    addLLVMIntConstant(out, func->getModule(), fcmpInst->getType(), "1");
                } else {
                    out << " = if (" << getWhy3TheoryName(fcmpInst->getOperand(0)->getType()) << ".";
                   
                    switch (fcmpInst->getPredicate()) {
                        case FCmpInst::FCMP_OEQ: {
                            out << "oeq";
                            break;
                        }
                        case FCmpInst::FCMP_OGT: {
                            out << "ogt";
                            break;
                        }
                        case FCmpInst::FCMP_OGE: {
                            out << "oge";
                            break;
                        }
                        case FCmpInst::FCMP_OLT: {
                            out << "olt";
                            break;
                        }
                        case FCmpInst::FCMP_OLE: {
                            out << "ole";
                            break;
                        }
                        case FCmpInst::FCMP_ONE: {
                            out << "one";
                            break;
                        }
                        case FCmpInst::FCMP_ORD: {
                            out << "ord";
                            break;
                        }
                        case FCmpInst::FCMP_UEQ: {
                            out << "ueq";
                            break;
                        }
                        case FCmpInst::FCMP_UGT: {
                            out << "ugt";
                            break;
                        }
                        case FCmpInst::FCMP_UGE: {
                            out << "uge";
                            break;
                        }
                        case FCmpInst::FCMP_ULT: {
                            out << "ult";
                            break;
                        }
                        case FCmpInst::FCMP_ULE: {
                            out << "ule";
                            break;
                        }
                        case FCmpInst::FCMP_UNE: {
                            out << "une";
                            break;
                        }
                        case FCmpInst::FCMP_UNO: {
                            out << "uno";
                            break;
                        }
                        default: {
                            throw whyr_exception("Internal error: Undefined predicate value for fcmp", NULL, new NodeSource(func, inst));
                        }
                    }
                    
                    out << " ";
                    addOperand(out, func->getModule(), fcmpInst->getOperand(0), func);
                    out << " ";
                    addOperand(out, func->getModule(), fcmpInst->getOperand(1), func);
                    out << ") then ";
                    addLLVMIntConstant(out, func->getModule(), fcmpInst->getType(), "1");
                    out << " else ";
                    addLLVMIntConstant(out, func->getModule(), fcmpInst->getType(), "0");
                }
                break;
            }
            case Instruction::OtherOps::Select: {
                addOperand(out, func->getModule(), inst, func);
                out << " = if ";
                addOperand(out, func->getModule(), inst->getOperand(0), func);
                out << " = ";
                addLLVMIntConstant(out, func->getModule(), inst->getOperand(0)->getType(), "1");
                out << " then ";
                addOperand(out, func->getModule(), inst->getOperand(1), func);
                out <<" else ";
                addOperand(out, func->getModule(), inst->getOperand(2), func);
                break;
            }
            case Instruction::OtherOps::Call: {
                CallInst* callInst = cast<CallInst>(inst);
                Function* calledFuncRaw = callInst->getCalledFunction();
                AnnotatedFunction* calledFunc = func->getModule()->getFunction(calledFuncRaw);
                string calleeTheoryName = getWhy3TheoryName(calledFunc) + "_call_" + getTempName(inst);
                
                if (!combineGoals && (goalInst != inst || goalExpr != calledFunc->getRequiresClause())) {
                    out << calleeTheoryName << ".F.function_requires -> ";
                }
                
                if (!calledFuncRaw->getReturnType()->isVoidTy()) {
                    addOperand(out, func->getModule(), inst, func);
                    out << " = " << calleeTheoryName << ".F.ret_val -> ";
                }
                out << calleeTheoryName << ".F.function_ensures";
                out << " -> " << getWhy3StatepointBefore(func, inst->getNextNode()) << " = " << calleeTheoryName << ".F.exit_state";
                break;
            }
            case Instruction::TermOps::Unreachable:
            case Instruction::OtherOps::PHI: {
                // instrution is a nop
                out << "true";
                break;
            }
            default: {
                // give a warning
                if (func->getModule()->getSettings()) {
                    func->getModule()->getSettings()->warnings.push_back(whyr_warning(("Unknown opcode; instruction ignored"), NULL, new NodeSource(func, inst)));
                } else {
                    // Can't warn. We have no output channel to do it in without crashing.
                }
                
                // make instruction a no-op
                out << "true";
            }
        }
        out << ")" << endl;
        
        switch (inst->getOpcode()) {
            default: {
                // do nothing
            }
        }
    }
    
    void addBlock(ostream &out, AnnotatedFunction* func, BasicBlock* block, LogicExpression* goalExpr, Instruction* goalInst) {
        for (BasicBlock::iterator ii = block->begin(); ii != block->end(); ii++) {
            addInstruction(out, func, &*ii, goalExpr, goalInst);
        }
        
        // gather imports for assert/assume
        ostringstream clause_stream;
        bool combineGoals = (func->getModule()->getSettings() && func->getModule()->getSettings()->combineGoals);
        for (BasicBlock::iterator ii = block->begin(); ii != block->end(); ii++) {
            AnnotatedInstruction* inst = func->getAnnotatedInstruction(&*ii);
            if (inst) {
                TypeInfo info;
                Why3Data data;
                data.module = func->getModule();
                data.source = new NodeSource(func, &*ii);
                data.info = &info;
                data.statepoint = getWhy3StatepointBefore(func, &*ii);
                
                if (inst->getAssumeClause()) {
                    inst->getAssumeClause()->toWhy3(clause_stream, data);
                    
                    for (unordered_set<string>::iterator ii = data.importsNeeded.begin(); ii != data.importsNeeded.end(); ii++) {
                        out << "    use import " << *ii << endl;
                    }
                }
                
                if (inst->getAssertClause()) {
                    inst->getAssertClause()->toWhy3(clause_stream, data);
                    
                    for (unordered_set<string>::iterator ii = data.importsNeeded.begin(); ii != data.importsNeeded.end(); ii++) {
                        out << "    use import " << *ii << endl;
                    }
                }
            }
        }
        
        out << "    axiom " << getWhy3BlockName(func, block) << ": " << getWhy3BlockName(func, block) << " = (";
        
        bool first = true;
        unsigned parens = 1;
        for (BasicBlock::iterator ii = block->begin(); ii != block->end(); ii++) {
            // add implication
            if (first) {
                first = false;
            } else {
                out << " -> ";
            }
            
            // add assume/assert if available
            AnnotatedInstruction* inst = func->getAnnotatedInstruction(&*ii);
            if (inst) {
                TypeInfo info;
                Why3Data data;
                data.module = func->getModule();
                data.source = new NodeSource(func, &*ii);
                data.info = &info;
                data.statepoint = getWhy3StatepointBefore(func, &*ii);
                
                if (inst->getAssumeClause()) {
                    inst->getAssumeClause()->toWhy3(out, data);
                    out << " -> ";
                }
                
                if (inst->getAssertClause()) {
                    if (combineGoals || (goalInst == &*ii && goalExpr == inst->getAssertClause())) {
                        out << "(";
                        inst->getAssertClause()->toWhy3(out, data);
                        out << " /\\ (";
                        parens+=2;
                    } else {
                        inst->getAssertClause()->toWhy3(out, data);
                        out << " -> ";
                    }
                }
            }
            
            // add requires clause for function calls, if needed
            if (goalInst == &*ii && isa<CallInst>(&*ii)) {
                CallInst* callInst = cast<CallInst>(&*ii);
                Function* calledFuncRaw = callInst->getCalledFunction();
                AnnotatedFunction* calledFunc = func->getModule()->getFunction(calledFuncRaw);
                string calleeTheoryName = getWhy3TheoryName(calledFunc) + "_call_" + getTempName(&*ii);
                
                if (combineGoals || (calledFunc->getRequiresClause() && goalExpr == calledFunc->getRequiresClause())) {
                    TypeInfo info;
                    Why3Data data;
                    data.module = func->getModule();
                    data.source = new NodeSource(func, &*ii);
                    data.info = &info;
                    data.statepoint = getWhy3StatepointBefore(func, &*ii);
                    data.calleeTheoryName = calleeTheoryName.c_str();
                    
                    out << "(";
                    calledFunc->getRequiresClause()->toWhy3(out, data);
                    out << " /\\ (";
                    parens+=2;
                }
            }
            
            // add block predicate
            out << getWhy3StatementName(func, &*ii);
        }
        
        // add parens. Only 1 if no asserts.
        for (unsigned i = 0; i < parens; i++) {
            out << ")";
        }
        out << endl;
    }
    
    void addFunction(ostream &out, AnnotatedFunction* func) {
        addGoal(out, func, getWhy3TheoryName(func), NULL, NULL);
    }
    
    void addGoal(ostream &out, AnnotatedFunction* func, string theoryName, LogicExpression* goalExpr, Instruction* goalInst) {
        out << "theory " << theoryName << endl;
        TypeInfo info;
        getTypeInfo(info, func);
        addImports(out, new NodeSource(func), info);
        
        // add statepoints
        out << "    use import State" << endl;
        out << "    use import Globals" << endl;
        if (func->rawIR()->isDeclaration()) {
            out << "    constant entry_state : state" << endl;
        } else {
            for (unordered_set<string>::iterator ii = info.statepoints.begin(); ii != info.statepoints.end(); ii++) {
                out << "    constant " << *ii << " : state" << endl;
            }
        }
        out << "    constant exit_state : state" << endl;
        // add constants
        for (unordered_set<Value*>::iterator ii = info.locals.begin(); ii != info.locals.end(); ii++) {
            out << "    constant " << getWhy3VarName(*ii) << " : " << getWhy3FullName((*ii)->getType()) << endl;
        }
        if (!func->rawIR()->getReturnType()->isVoidTy()) {
            out << "    constant ret_val : " << getWhy3FullName(func->rawIR()->getReturnType()) << endl;
        }
        // add block predicates
        for (Function::iterator ii = func->rawIR()->begin(); ii != func->rawIR()->end(); ii++) {
            out << "    predicate " << getWhy3BlockName(func, &*ii) << endl;
        }
        
        // add requires clause
        if (func->getRequiresClause()) {
            ostringstream clause_stream;
            Why3Data data;
            data.module = func->getModule();
            data.source = new NodeSource(func);
            data.info = &info;
            data.statepoint = "entry_state";
            
            func->getRequiresClause()->toWhy3(clause_stream, data);
            for (unordered_set<string>::iterator ii = data.importsNeeded.begin(); ii != data.importsNeeded.end(); ii++) {
                out << "    use import " << *ii << endl;
            }
            out << "    predicate function_requires = " << clause_stream.str();
            
            out << endl;
        } else {
            out << "    predicate function_requires = true" << endl;
        }
        
        // add ensures clause
        if (func->getEnsuresClause()) {
            ostringstream clause_stream;
            Why3Data data;
            data.module = func->getModule();
            data.source = new NodeSource(func);
            data.info = &info;
            data.statepoint = "exit_state";
            
            func->getEnsuresClause()->toWhy3(clause_stream, data);
            for (unordered_set<string>::iterator ii = data.importsNeeded.begin(); ii != data.importsNeeded.end(); ii++) {
                out << "    use import " << *ii << endl;
            }
            out << "    predicate function_ensures = " << clause_stream.str();
            
            out << endl;
        } else {
            out << "    predicate function_ensures = true" << endl;
        }
        
        // add the exact execution predicate
        if (func->rawIR()->isDeclaration()) {
            out << "    predicate execute = function_ensures" << endl;
        } else {
            out << "    predicate execute = " << getWhy3BlockName(func, &func->rawIR()->getEntryBlock()) << endl;
        }
        
        // build each block
        for (Function::iterator ii = func->rawIR()->begin(); ii != func->rawIR()->end(); ii++) {
            addBlock(out, func, &*ii, goalExpr, goalInst);
        }
        
        // add the goal, if we have one
        if ((func->getModule()->getSettings() && func->getModule()->getSettings()->combineGoals) || goalExpr) {
            out << "    goal " << theoryName << ": function_requires -> execute" << endl;
        }
        out << "end" << endl << endl;
    }
    
    void addCommonIntType(ostream &out, AnnotatedModule* module) {
        out << "theory LLVMInt" << endl;
        if (!module->getSettings() || module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_INT) {
            out << "    use import int.Int" << endl;
            out << "    use import int.ComputerDivision" << endl;
            out << "    use import bv.Pow2int" << endl;
            
            out << "    type t = int" << endl;
            out << "    constant size : int" << endl;
            out << "    constant max_int : int" << endl;
            out << "    constant two_power_size : int" << endl;
            out << "    constant undef : t" << endl;
            
            out << "    namespace import BV" << endl;
            out << "        clone import bv.BV_Gen as GEN with" << endl;
            out << "            constant size = size," << endl;
            out << "            constant two_power_size = two_power_size," << endl;
            out << "            constant max_int = max_int" << endl;
            out << "    end" << endl;
            
            out << "    function to_uint (i:t) :int = i" << endl;
            out << "    function to_int (i:t) :int = i" << endl;
            out << "    function of_int (i:int) :t = i" << endl;
            
            out << "    function add (a:t) (b:t) :t = a + b" << endl;
            out << "    function sub (a:t) (b:t) :t = a - b" << endl;
            out << "    function mul (a:t) (b:t) :t = a * b" << endl;
            
            out << "    function sdiv (a:t) (b:t) :t = (div a b)" << endl;
            out << "    function udiv (a:t) (b:t) :t = (div a b)" << endl;
            out << "    function srem (a:t) (b:t) :t = (mod a b)" << endl;
            out << "    function urem (a:t) (b:t) :t = (mod a b)" << endl;
            
            out << "    predicate ult (a:t) (b:t) = a < b" << endl;
            out << "    predicate ule (a:t) (b:t) = a <= b" << endl;
            out << "    predicate ugt (a:t) (b:t) = a > b" << endl;
            out << "    predicate uge (a:t) (b:t) = a >= b" << endl;
            
            out << "    predicate slt (a:t) (b:t) = a < b" << endl;
            out << "    predicate sle (a:t) (b:t) = a <= b" << endl;
            out << "    predicate sgt (a:t) (b:t) = a > b" << endl;
            out << "    predicate sge (a:t) (b:t) = a >= b" << endl;
            
            out << "    function bw_and (a:t) (b:t) :t = (BV.GEN.to_int (BV.GEN.bw_and (BV.GEN.of_int a) (BV.GEN.of_int b)))" << endl;
            out << "    function bw_or (a:t) (b:t) :t = (BV.GEN.to_int (BV.GEN.bw_or (BV.GEN.of_int a) (BV.GEN.of_int b)))" << endl;
            out << "    function bw_xor (a:t) (b:t) :t = (BV.GEN.to_int (BV.GEN.bw_xor (BV.GEN.of_int a) (BV.GEN.of_int b)))" << endl;
            out << "    function bw_not (a:t) :t = (BV.GEN.to_int (BV.GEN.bw_not (BV.GEN.of_int a)))" << endl;
            
            out << "    function lsl (a:t) (n:int) :t = (a * (pow2 n))" << endl;
            out << "    function lsr (a:t) (n:int) :t = (div a (pow2 n))" << endl;
            out << "    function asr (a:t) (n:int) :t = (div a (pow2 n))" << endl;
        } else if (module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_BV) {
            out << "    use import int.Int" << endl;
            out << "    use import int.ComputerDivision" << endl;
            
            out << "    type t" << endl;
            out << "    clone export bv.BV_Gen with type t = t" << endl;
            
            out << "    constant undef : t" << endl;
            out << "    function sdiv (a:t) (b:t) :t = (of_int (div (to_int a) (to_int b)))" << endl;
            out << "    function srem (a:t) (b:t) :t = (of_int (mod (to_int a) (to_int b)))" << endl;
        }
        out << "end" << endl << endl;
    }
    
    void addIntType(ostream &out, AnnotatedModule* module, IntegerType* type) {
        string bits_string = to_string(type->getIntegerBitWidth());
        out << "theory " << getWhy3TheoryName(type) << endl;
        out << "    use import int.Int" << endl;
        out << "    constant size : int = " << type->getIntegerBitWidth() << endl;
        out << "    constant max_int : int = 0b";
        for (unsigned i = 0; i < type->getIntegerBitWidth(); i++) {
            out << "1";
        }
        out << endl;
        out << "    constant two_power_size : int = 0b1";
        for (unsigned i = 0; i < type->getIntegerBitWidth(); i++) {
            out << "0";
        }
        out << endl;
        
        if (!module->getSettings() || module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_INT) {
            out << "    clone export LLVMInt with" << endl;
            out << "        constant size = size," << endl;
            out << "        constant two_power_size = two_power_size," << endl;
            out << "        constant max_int = max_int" << endl;
            out << "    type i" << bits_string << " = t" << endl;
        } else if (module->getSettings()->why3IntMode == WhyRSettings::WHY3_INT_MODE_BV) {
            out << "    type i" << bits_string << endl;
            out << "    clone export LLVMInt with" << endl;
            out << "        type t = i" << bits_string << "," << endl;
            out << "        constant size = size," << endl;
            out << "        constant two_power_size = two_power_size," << endl;
            out << "        constant max_int = max_int" << endl;
        }
        
        out << "end" << endl << endl;
    }
    
    void addCommonPtrType(ostream &out, AnnotatedModule* module) {
        int ptrBits = module->rawIR()->getDataLayout().getPointerSizeInBits(0); // TODO: address spaces...
        out << "theory LLVMPtr" << endl;
        
        if (!module->getSettings() || module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DEFAULT) {
            out << "    use import int.Int" << endl;
            out << "    use export Pointer" << endl;
            
            out << "    use import I" << ptrBits << endl;
            out << "    constant size : int = " << ptrBits << endl;
            out << "    constant elem_size : int" << endl;
            
            out << "    type t" << endl;
            out << "    type p = (pointer t)" << endl;
            
            out << "    axiom size: forall a : p. (bits a) = elem_size" << endl;
            
            out << "    function to_ptrint p :I" << ptrBits << ".i" << ptrBits << endl;
            out << "    function of_ptrint I" << ptrBits << ".i" << ptrBits << " :p" << endl;
            out << "    axiom to_from_ptrint: forall p. (of_ptrint (to_ptrint p)) = p" << endl;
            out << "    axiom offset_ptrint: forall p i. (of_ptrint (I" << ptrBits << ".add i (to_ptrint p))) = (offset_pointer p (I" << ptrBits << ".to_int (I" << ptrBits << ".mul i (bits p))))" << endl;
        } else if (module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DUMMY) {
            out << "    use import Pointer" << endl;
            out << "    use import State" << endl;
            out << "    use import I" << ptrBits << endl;
            
            out << "    type t" << endl;
            
            out << "    constant size : int = " << ptrBits << endl;
            out << "    constant elem_size : int" << endl;
            
            out << "    constant null : pointer" << endl;
            
            out << "    function load state pointer :t" << endl;
            out << "    function store state pointer t :state" << endl;
            out << "    function offset pointer int :pointer" << endl;
            out << "    function to_ptrint pointer :I" << ptrBits << ".i" << ptrBits << endl;
            out << "    function of_ptrint I" << ptrBits << ".i" << ptrBits << " :pointer" << endl;
        }
        
        out << "end" << endl << endl;
    }
    
    void addPtrType(ostream &out, AnnotatedModule* module, PointerType* type) {
        out << "theory " << getWhy3TheoryName(type) << endl;
        
        if (!module->getSettings() || module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DEFAULT) {
            out << "    use import int.Int" << endl;
            out << "    use import " << getWhy3TheoryName(type->getElementType()) << endl;
            
            out << "    constant elem_size : int = " << getWhy3TheoryName(type->getElementType()) << ".size" << endl;
            out << "    clone export LLVMPtr with" << endl;
            out << "        constant elem_size = elem_size," << endl;
            out << "        type t = " << getWhy3FullName(type->getElementType()) << endl;
            
            out << "    type " << getWhy3TypeName(type) << " = p" << endl;
        } else if (module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DUMMY) {
            out << "    use import Pointer" << endl;
            out << "    use import " << getWhy3TheoryName(type->getElementType()) << endl;
            
            out << "    constant elem_size : int = " << getWhy3TheoryName(type->getElementType()) << ".size" << endl;
            
            out << "    clone export LLVMPtr with" << endl;
            out << "        constant elem_size = elem_size," << endl;
            out << "        type t = " << getWhy3FullName(type->getElementType()) << endl;
            
            out << "    type " << getWhy3TypeName(type) << " = pointer" << endl;
        }
        
        out << "end" << endl << endl;
    }
    
    void addCommonFloatType(ostream &out, AnnotatedModule* module) {
        out << "theory LLVMFloat" << endl;
        if (!module->getSettings() || module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_REAL) {
            out << "    use import int.Int" << endl;
            out << "    use import real.Real" << endl;
            out << "    use import CustomTruncate" << endl;
            out << "    use import real.FromInt" << endl;
            out << "    use import floating_point.Rounding" << endl;
            
            out << "    type t = real" << endl;
            out << "    type mode = Rounding.mode" << endl;
            
            out << "    constant size : int" << endl;
            
            out << "    constant min : real" << endl;
            out << "    constant max : real" << endl;
            out << "    constant max_representable_integer : int" << endl;
            
            out << "    function to_real mode (x:t) :real = x" << endl;
            out << "    function of_real mode (x:real) :t = x" << endl;
            
            out << "    predicate is_finite (x:t) = true" << endl;
            out << "    predicate is_infinite (x:t) = false" << endl;
            out << "    predicate is_NaN (x:t) = false" << endl;
            
            out << "    function fadd (m:mode) (a:t) (b:t) :t = a + b" << endl;
            out << "    function fsub (m:mode) (a:t) (b:t) :t = a - b" << endl;
            out << "    function fmul (m:mode) (a:t) (b:t) :t = a * b" << endl;
            out << "    function fdiv (m:mode) (a:t) (b:t) :t = a / b" << endl;
            
            out << "    function frem (m:mode) (a:t) (b:t) :t = a - (from_int (truncate (a / b))) * b" << endl;
            
            out << "    predicate oeq (a:t) (b:t) = (a = b)" << endl;
            out << "    predicate ogt (a:t) (b:t) = (a > b)" << endl;
            out << "    predicate oge (a:t) (b:t) = (a >= b)" << endl;
            out << "    predicate olt (a:t) (b:t) = (a < b)" << endl;
            out << "    predicate ole (a:t) (b:t) = (a <= b)" << endl;
            out << "    predicate one (a:t) (b:t) = (a <> b)" << endl;
            out << "    predicate ord (a:t) (b:t) = true" << endl;
            out << "    predicate ueq (a:t) (b:t) = (a = b)" << endl;
            out << "    predicate ugt (a:t) (b:t) = (a > b)" << endl;
            out << "    predicate uge (a:t) (b:t) = (a >= b)" << endl;
            out << "    predicate ult (a:t) (b:t) = (a < b)" << endl;
            out << "    predicate ule (a:t) (b:t) = (a <= b)" << endl;
            out << "    predicate une (a:t) (b:t) = (a <> b)" << endl;
            out << "    predicate uno (a:t) (b:t) = true" << endl;
        } else if (module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_FP) {
            out << "    use import int.Int" << endl;
            out << "    use import real.Real" << endl;
            out << "    use import CustomTruncate" << endl;
            out << "    use import real.FromInt" << endl;
            
            out << "    type t" << endl;
            
            out << "    constant size : int" << endl;
            
            out << "    constant min : real" << endl;
            out << "    constant max : real" << endl;
            out << "    constant max_representable_integer : int" << endl;
            
            out << "    clone import floating_point.GenFloatSpecFull as FP with" << endl;
            out << "        type t = t," << endl;
            out << "        constant min = min," << endl;
            out << "        constant max = max," << endl;
            out << "        constant max_representable_integer = max_representable_integer" << endl;
            out << "    type mode = FP.Rounding.mode" << endl;
            
            out << "    function to_real (m:mode) (x:t) :real" << endl;
            out << "    axiom to_real_exact: forall m x r. r = (to_real m x) <-> r = (FP.exact x)" << endl;
            out << "    axiom to_real_value: forall m x r. r = (to_real m x) <-> (FP.round m r) = (FP.value x)" << endl;
            out << "    function of_real (m:mode) (x:real) :t = (FP.round_logic m x)" << endl;

            out << "    predicate is_finite (x:t) = (FP.is_finite x)" << endl;
            out << "    predicate is_infinite (x:t) = (FP.is_infinite x)" << endl;
            out << "    predicate is_NaN (x:t) = (FP.is_NaN x)" << endl;
            
            out << "    function fadd mode t t :t" << endl;
            out << "    axiom fadd: forall m :mode. forall a b r :t. (r = (fadd m a b)) <-> (FP.add_post m a b r)" << endl;
            out << "    function fsub mode t t :t" << endl;
            out << "    axiom fsub: forall m :mode. forall a b r :t. (r = (fsub m a b)) <-> (FP.sub_post m a b r)" << endl;
            out << "    function fmul mode t t :t" << endl;
            out << "    axiom fmul: forall m :mode. forall a b r :t. (r = (fmul m a b)) <-> (FP.mul_post m a b r)" << endl;
            out << "    function fdiv mode t t :t" << endl;
            out << "    axiom fdiv: forall m :mode. forall a b r :t. (r = (fdiv m a b)) <-> (FP.div_post m a b r)" << endl;
            
            out << "    function frem (m:mode) (a:t) (b:t) :t = (fsub m a (fmul m (of_real m (from_int (truncate (to_real m (fdiv m a b))))) b))" << endl;
            
            out << "    predicate oeq (a:t) (b:t) = (FP.eq a b)" << endl;
            out << "    predicate ogt (a:t) (b:t) = (FP.gt a b)" << endl;
            out << "    predicate oge (a:t) (b:t) = (FP.ge a b)" << endl;
            out << "    predicate olt (a:t) (b:t) = (FP.lt a b)" << endl;
            out << "    predicate ole (a:t) (b:t) = (FP.le a b)" << endl;
            out << "    predicate one (a:t) (b:t) = (FP.ne a b)" << endl;
            out << "    predicate ord (a:t) (b:t) = (FP.is_not_NaN a) /\\ (FP.is_not_NaN b)" << endl;
            out << "    predicate ueq (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.eq a b)" << endl;
            out << "    predicate ugt (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.gt a b)" << endl;
            out << "    predicate uge (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.ge a b)" << endl;
            out << "    predicate ult (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.lt a b)" << endl;
            out << "    predicate ule (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.le a b)" << endl;
            out << "    predicate une (a:t) (b:t) = (FP.is_NaN a) \\/ (FP.is_NaN b) \\/ (FP.ne a b)" << endl;
            out << "    predicate uno (a:t) (b:t) = (FP.is_not_NaN a) \\/ (FP.is_not_NaN b)" << endl;
        }
        out << "end" << endl << endl;
    }
    
    void addFloatType(ostream &out, AnnotatedModule* module, Type* type) {
        // TODO: 16-bit floats, 80-bit floats, 128-bit floats...
        out << "theory " << getWhy3TheoryName(type) << endl;
        
        out << "    use import int.Int" << endl;
        out << "    use import real.Real" << endl;
        
        out << "    constant size : int = ";
        switch (type->getTypeID()) {
            case llvm::Type::TypeID::FloatTyID: {
                out << "32"; break;
            }
            case llvm::Type::TypeID::DoubleTyID: {
                out << "64"; break;
            }
            default: throw whyr_exception("Internal error: Unknown float type");
        }
        out << endl;
        
        out << "    constant min : real = ";
        switch (type->getTypeID()) {
            case llvm::Type::TypeID::FloatTyID: {
                out << "0x1p-149"; break;
            }
            case llvm::Type::TypeID::DoubleTyID: {
                out << "0x1p-1074"; break;
            }
            default: throw whyr_exception("Internal error: Unknown float type");
        }
        out << endl;
        
        out << "    constant max : real = ";
        switch (type->getTypeID()) {
            case llvm::Type::TypeID::FloatTyID: {
                out << "0x1.FFFFFEp127"; break;
            }
            case llvm::Type::TypeID::DoubleTyID: {
                out << "0x1.FFFFFFFFFFFFFp1023"; break;
            }
            default: throw whyr_exception("Internal error: Unknown float type");
        }
        out << endl;
        
        out << "    constant max_representable_integer : int = ";
        switch (type->getTypeID()) {
            case llvm::Type::TypeID::FloatTyID: {
                out << "16777216"; break;
            }
            case llvm::Type::TypeID::DoubleTyID: {
                out << "9007199254740992"; break;
            }
            default: throw whyr_exception("Internal error: Unknown float type");
        }
        out << endl;
        
        if (!module->getSettings() || module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_REAL) {
            out << "    clone export LLVMFloat with" << endl;
            out << "        constant size = size," << endl;
            out << "        constant min = min," << endl;
            out << "        constant max = max," << endl;
            out << "        constant max_representable_integer = max_representable_integer" << endl;
            out << "    type " << getWhy3TypeName(type) << " = t" << endl;
        } else if (module->getSettings()->why3FloatMode == WhyRSettings::WHY3_FLOAT_MODE_FP) {
            out << "    type " << getWhy3TypeName(type) << endl;
            out << "    clone export LLVMFloat with" << endl;
            out << "        type t = " << getWhy3TypeName(type) << "," << endl;
            out << "        constant size = size," << endl;
            out << "        constant min = min," << endl;
            out << "        constant max = max," << endl;
            out << "        constant max_representable_integer = max_representable_integer" << endl;
        }
        
        out << "end" << endl << endl;
    }
    
    void addCommonArrayType(ostream &out, AnnotatedModule* module) {
        out << "theory LLVMArray" << endl;
        out << "    use export map.Map" << endl;
        out << "    use import int.Int" << endl;
        
        out << "    type t" << endl;
        out << "    type a = (map int t)" << endl;
        
        out << "    constant elem_num : int" << endl;
        out << "    constant size : int" << endl;
        out << "    constant elem_size : int" << endl;
        
        out << "    constant any_array : a" << endl;
        out << "end" << endl << endl;
    }
    
    void addArrayType(ostream &out, AnnotatedModule* module, ArrayType* type) {
        out << "theory " << getWhy3TheoryName(type) << endl;
        out << "    use import " << getWhy3TheoryName(type->getArrayElementType()) << endl;
        out << "    use import int.Int" << endl;
        
        out << "    constant elem_num : int = " << type->getArrayNumElements() << endl;
        out << "    constant size : int = " << getWhy3TheoryName(type->getArrayElementType()) << ".size * elem_num" << endl;
        out << "    constant elem_size : int = " << getWhy3TheoryName(type->getArrayElementType()) << ".size" << endl;
        
        out << "    clone export LLVMArray with" << endl;
        out << "        constant elem_num = elem_num," << endl;
        out << "        constant size = size," << endl;
        out << "        constant elem_size = elem_size," << endl;
        out << "        type t = " << getWhy3FullName(type->getArrayElementType()) << endl;
        out << "    type " << getWhy3TypeName(type) << " = a" << endl;
        out << "end" << endl << endl;
    }
    
    void addCommonStructType(ostream &out, AnnotatedModule* module) {
        // do nothing. Currently, no memory models require we have any common code.
    }
    
    void addStructType(ostream &out, AnnotatedModule* module, StructType* type) {
        // find the imports we need
        unordered_set<Type*> imports;
        for (unsigned i = 0; i < type->getStructNumElements(); i++) {
            imports.insert(type->getStructElementType(i));
        }
        
        // add the theory
        out << "theory " << getWhy3TheoryName(type) << endl;
        for (unordered_set<Type*>::iterator ii = imports.begin(); ii != imports.end(); ii++) {
            out << "    use import " << getWhy3TheoryName(*ii) << endl;
        }
        out << "    constant size : int = " << module->rawIR()->getDataLayout().getStructLayout(type)->getSizeInBits() << endl;
        out << "    type " << getWhy3TypeName(type) << " = {" << endl;
        for (unsigned i = 0; i < type->getStructNumElements(); i++) {
            Type* elem = type->getStructElementType(i);
            out << "        " << getWhy3StructFieldName(module, type, i) << " : " << getWhy3FullName(elem) << ";" << endl;
        }
        out << "    }" << endl;
        out << "end" << endl << endl;
    }
    
    void addCommonVectorType(ostream &out, AnnotatedModule* module) {
        out << "theory LLVMVector" << endl;
        out << "    use export map.Map" << endl;
        out << "    use import int.Int" << endl;
        
        out << "    type t" << endl;
        out << "    type v = (map int t)" << endl;
        
        out << "    constant elem_num : int" << endl;
        out << "    constant size : int" << endl;
        out << "    constant elem_size : int" << endl;
        
        out << "    constant any_vector : v" << endl;
        out << "end" << endl << endl;
    }
    
    void addVectorType(ostream &out, AnnotatedModule* module, VectorType* type) {
        out << "theory " << getWhy3TheoryName(type) << endl;
        out << "    use import " << getWhy3TheoryName(type->getVectorElementType()) << endl;
        out << "    use import bool.Bool" << endl;
        out << "    use import int.Int" << endl;
        
        out << "    constant elem_num : int = " << type->getVectorNumElements() << endl;
        out << "    constant size : int = " << getWhy3TheoryName(type->getVectorElementType()) << ".size * elem_num" << endl;
        out << "    constant elem_size : int = " << getWhy3TheoryName(type->getVectorElementType()) << ".size" << endl;
        
        out << "    clone export LLVMVector with" << endl;
        out << "        constant elem_num = elem_num," << endl;
        out << "        constant size = size," << endl;
        out << "        constant elem_size = elem_size," << endl;
        out << "        type t = " << getWhy3FullName(type->getVectorElementType()) << endl;
        out << "    type " << getWhy3TypeName(type) << " = v" << endl;
        
        // add the vector versions of operators that can be done on the element type
        if (type->getVectorElementType()->isIntegerTy()) {
            out << "    type bool_vector = (map int bool)" << endl;
            out << "    constant any_bool_vector : bool_vector" << endl;
            out << "    type int_vector = (map int int)" << endl;
            out << "    constant any_int_vector : int_vector" << endl;
            
            static const list<string> mathOps({"add","sub","mul","udiv","sdiv","urem","srem", "bw_and", "bw_or", "bw_xor"});
            for (list<string>::const_iterator ii = mathOps.begin(); ii != mathOps.end(); ii++) {
                out << "    function " << *ii << " (a:v) (b:v) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << "." << *ii << " a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function bw_not (a:v) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".bw_not " << " a[" << i << "])]";
                }
                out << endl;
            }
            
            static const list<string> shiftOps({"lsl", "lsr", "asr"});
            for (list<string>::const_iterator ii = shiftOps.begin(); ii != shiftOps.end(); ii++) {
                out << "    function " << *ii << " (a:v) (b:int_vector) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << "." << *ii << " a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            static const list<string> compOps({"ugt","uge","ult","ule","sgt","sge","slt","sle"});
            for (list<string>::const_iterator ii = compOps.begin(); ii != compOps.end(); ii++) {
                out << "    function " << *ii << " (a:v) (b:v) :bool_vector = any_bool_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << "." << *ii << " a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function to_uint (a:v) :int_vector = any_int_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".to_uint a[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function to_int (a:v) :int_vector = any_int_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".to_int a[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function of_int (a:int_vector) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".of_int a[" << i << "])]";
                }
                out << endl;
            }
        } else if (type->getVectorElementType()->isFloatingPointTy()) {
            out << "    use import real.Real" << endl;
            
            out << "    type bool_vector = (map int bool)" << endl;
            out << "    constant any_bool_vector : bool_vector" << endl;
            out << "    type real_vector = (map int real)" << endl;
            out << "    constant any_real_vector : real_vector" << endl;
            
            static const list<string> mathOps({"fadd","fsub","fmul","fdiv","frem"});
            for (list<string>::const_iterator ii = mathOps.begin(); ii != mathOps.end(); ii++) {
                out << "    function " << *ii << " (m:Rounding.mode) (a:v) (b:v) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << "." << *ii << " m a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            static const list<string> compOps({"oeq","ogt","oge","olt","ole","one","ord","ueq","ugt","uge","ult","ule","une","uno"});
            for (list<string>::const_iterator ii = compOps.begin(); ii != compOps.end(); ii++) {
                out << "    function " << *ii << " (a:v) (b:v) :bool_vector = any_bool_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << "." << *ii << " a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function to_real (m:Rounding.mode) (a:v) :real_vector = any_real_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".to_real m a[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function of_real (m:Rounding.mode) (a:real_vector) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".of_real m a[" << i << "])]";
                }
                out << endl;
            }
        } else if (type->getVectorElementType()->isPointerTy()) {
            Type* ptrint = IntegerType::get(module->rawIR()->getContext(), module->rawIR()->getDataLayout().getPointerSizeInBits(0)); // TODO: address spaces...
            out << "    namespace import PTRINT use import " << getWhy3TheoryName(ptrint) << " end" << endl;
            
            out << "    type int_vector = (map int int)" << endl;
            out << "    constant any_int_vector : int_vector" << endl;
            out << "    type ptrint_vector = (map int PTRINT." << getWhy3FullName(ptrint) << ")" << endl;
            out << "    constant any_ptrint_vector : ptrint_vector" << endl;
            
            {
                out << "    function offset_pointer (a:v) (b:int_vector) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".offset_pointer a[" << i << "] b[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function to_ptrint (a:v) :ptrint_vector = any_ptrint_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".to_ptrint a[" << i << "])]";
                }
                out << endl;
            }
            
            {
                out << "    function of_ptrint (a:ptrint_vector) :v = any_vector";
                for (unsigned i = 0; i < type->getVectorNumElements(); i++) {
                    out << "[" << i << " <- (" << getWhy3TheoryName(type->getVectorElementType()) << ".of_ptrint a[" << i << "])]";
                }
                out << endl;
            }
        }
        
        out << "end" << endl << endl;
    }
    
    void addDerivedType(ostream &out, AnnotatedModule* module, Type* type) {
        if (type->isPointerTy()) {
            addPtrType(out, module, cast<PointerType>(type));
        } else if (type->isArrayTy()) {
            addArrayType(out, module, cast<ArrayType>(type));
        } else if (type->isStructTy()) {
            addStructType(out, module, cast<StructType>(type));
        } else if (type->isVectorTy()) {
            addVectorType(out, module, cast<VectorType>(type));
        }
    }
    
    void addBlockAddressType(ostream &out, AnnotatedModule* module) {
        Type* i8p = PointerType::get(IntegerType::get(module->rawIR()->getContext(), 8), 0);
        
        out << "theory BlockAddress" << endl;
        out << "    use import bool.Bool" << endl;
        out << "    use import " << getWhy3TheoryName(i8p) << endl;
        out << "    function store_baddr bool :" << getWhy3FullName(i8p) << endl;
        out << "    predicate load_baddr " << getWhy3FullName(i8p) << endl;
        out << "    axiom store_load_baddr: forall p. (load_baddr (store_baddr p)) = p" << endl;
        out << "    axiom baddr_null_test: forall p. (store_baddr p) <> " << getWhy3TheoryName(i8p) << ".null" << endl;
        out << "end" << endl << endl;
    }
    
    void addGlobals(ostream &out, AnnotatedModule* module) {
        out << "theory Globals" << endl;
        out << "    use import State" << endl;
        out << "    use import Pointer" << endl;
        out << "    use import Alloc" << endl;
        
        TypeInfo info;
        info.module = module;
        for (Module::GlobalListType::iterator ii = module->rawIR()->getGlobalList().begin(); ii != module->rawIR()->getGlobalList().end(); ii++) {
            getTypeInfo(info, ii->getType());
        }
        addImports(out, new NodeSource((AnnotatedFunction*)NULL), info); // FIXME: NodeSource?
        
        string lastState = "State.blank_state";
        for (Module::GlobalListType::iterator ii = module->rawIR()->getGlobalList().begin(); ii != module->rawIR()->getGlobalList().end(); ii++) {
            out << "    constant state_after_" << getWhy3GlobalName(&*ii) << " : state" << endl;
            out << "    constant " << getWhy3GlobalName(&*ii) << " : " << getWhy3FullName(ii->getType()) << endl;
            out << "    axiom " << getWhy3GlobalName(&*ii) << ": (state_after_" << getWhy3GlobalName(&*ii) << ", " << getWhy3GlobalName(&*ii) << ") = (alloc " << lastState << " " << getWhy3TheoryName(ii->getType()) << ".elem_size)";
        
            lastState = "state_after_" + getWhy3GlobalName(&*ii);
        }
        
        out << "    constant any_state : state = " << lastState << endl;
        
        lastState = "any_state";
        for (Module::GlobalListType::iterator ii = module->rawIR()->getGlobalList().begin(); ii != module->rawIR()->getGlobalList().end(); ii++) {
            out << "    constant state_init_" << getWhy3GlobalName(&*ii) << " : state = (" << getWhy3TheoryName(ii->getType()) << ".store " << lastState << " " << getWhy3GlobalName(&*ii) << " ";
            addOperand(out, module, ii->getInitializer());
            out << ")" << endl;
            
            lastState = "state_init_" + getWhy3GlobalName(&*ii);
        }
        
        out << "    constant init_state : state = " << lastState << endl;
        
        out << "end" << endl << endl;
    }
    
    static const string mem_model_default = R"(theory State
    use import map.Map
    use import int.Int
    use import bool.Bool
    
    type base_id = Int.int
    constant null_base : base_id = 0
    
    type mem_data
    type mem_space = {
        size : Int.int;
        data : mem_data;
        static : Bool.bool;
    }
    
    type state = {
        n_allocs : base_id;
        memory : (Map.map base_id mem_space);
    }
    
    constant init_memory : (Map.map base_id mem_space)
    constant blank_state : state = {
        n_allocs = 1;
        memory = init_memory;
    }
end

theory Pointer
    use import State
    use import map.Map
    use import int.Int
    
    type pointer 'a = {
        base : State.base_id;
        offset : Int.int;
    }
    
    constant null : pointer 'a = {
        base = State.null_base;
        offset = 0;
    }
    
    predicate valid_read (s:State.state) (p:(pointer 'a)) (i:Int.int) =
        let mem = (Map.get s.memory p.base) in
    (p.offset >= 0) /\ (p.offset + i >= 0) /\ (p.offset < mem.size) /\ (p.offset + i < mem.size)
    predicate valid_write (s:State.state) (p:(pointer 'a)) (i:Int.int) =
        let mem = (Map.get s.memory p.base) in
    (valid_read s p i) /\ (not mem.static)
    predicate separated (a:(pointer 'a)) (i:Int.int) (b:(pointer 'b)) (j:Int.int) =
    ((a.base <> b.base) \/ ((b.offset>a.offset\/a.offset>b.offset+j)/\(a.offset>b.offset\/b.offset>a.offset+i)))
    
    function bits (pointer 'a) : Int.int
    axiom bits_gt_0: forall p : (pointer 'a).
        bits p > 0
    
    function store_mem State.mem_data (pointer 'a) 'a :State.mem_data
    function load_mem State.mem_data (pointer 'a) :'a
    
    axiom store_then_load: forall m. forall p : (pointer 'a). forall v.
    (load_mem (store_mem m p v) p) = v
    axiom load_separation: forall a : (pointer 'a). forall b : (pointer 'b). forall m y.
    (separated a (bits a) b (bits b)) -> (load_mem (store_mem m b y) a) = (load_mem m a)
    
    function load (s:State.state) (p:(pointer 'a)) :'a =
        let mem = (Map.get s.memory p.base) in
    (load_mem mem.data p)
    function store (s:State.state) (p:(pointer 'a)) (v:'a) :State.state =
        let old_mem = (Map.get s.memory p.base) in
        let new_mem = {old_mem with data = (store_mem old_mem.data p v);} in
    {s with memory = (Map.set s.memory p.base new_mem);}
    
    function cast (p:(pointer 'a)) :(pointer 'b) =
    { base = p.base; offset = p.offset; }

    function offset_pointer (p:(pointer 'a)) (n:int) :(pointer 'a) =
    { base = p.base; offset = p.offset + n; }
end

theory Alloc
    use import State
    use import Pointer
    use import int.Int
    use import map.Map
    
    constant blank_mem_data : State.mem_data
    
    function alloc (s:State.state) (i:Int.int) :(State.state,(Pointer.pointer 'a)) =
        let new_mem = ({
            size = i;
            data = blank_mem_data;
            static = false;
        }:mem_space) in
        let new_map = (Map.set s.memory s.n_allocs new_mem) in
        let new_s = {s with
            memory = new_map;
            n_allocs = s.n_allocs + 1;
        } in
        let new_p = {
            base = s.n_allocs;
            offset = 0;
        } in
    (new_s,new_p)
end
)";

    static const string mem_model_dummy = R"(theory State
    type state
    constant blank_state : state
end

theory Pointer
    use import State
    use import int.Int
    
    type pointer
    predicate valid_read state pointer int
    predicate valid_write state pointer int
    predicate separated pointer int pointer int
    function cast (p:pointer) :pointer
end

theory Alloc
    use import State
    use import Pointer
    use import int.Int
    
    function alloc state int :(state,pointer)
end
)";
    
    void addMemoryModel(ostream &out, AnnotatedModule* module) {
        if (!module->getSettings() || module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DEFAULT) {
            out << mem_model_default << endl;
        } else if (module->getSettings()->why3MemModel == WhyRSettings::WHY3_MEM_MODEL_DUMMY) {
            out << mem_model_dummy << endl;
        }
    }
    
    void getCorrectDerivedTypeOrder(list<Type*> &types, TypeInfo &info, AnnotatedModule* mod) {
        unordered_set<Type*> notSeen;
        notSeen.insert(info.ptrTypes.begin(), info.ptrTypes.end());
        notSeen.insert(info.arrayTypes.begin(), info.arrayTypes.end());
        notSeen.insert(info.structTypes.begin(), info.structTypes.end());
        notSeen.insert(info.vectorTypes.begin(), info.vectorTypes.end());
        
        // while there's still more nodes in need of calculation; that is, if the state changed since last iteration...
        bool changed;
        do {
            changed = false;
            
            for (unordered_set<Type*>::iterator ii = notSeen.begin(); ii != notSeen.end(); /* ii++ is below */) {
                
                // if all dependancy nodes have been seen, then we are ready to add to the list.
                // Note that this means nodes with no dependencies automatically pass the test.
                bool allSeen = true;
                if ((*ii)->isPointerTy()) {
                    if (notSeen.find((*ii)->getPointerElementType()) != notSeen.end()) {
                        allSeen = false;
                    }
                } else if ((*ii)->isArrayTy()) {
                    if (notSeen.find((*ii)->getArrayElementType()) != notSeen.end()) {
                        allSeen = false;
                    }
                } else if ((*ii)->isStructTy()) {
                    for (unsigned i = 0; i < (*ii)->getStructNumElements(); i++) {
                        if (notSeen.find((*ii)->getStructElementType(i)) != notSeen.end()) {
                            allSeen = false;
                            break;
                        }
                    }
                } else if ((*ii)->isVectorTy()) {
                    if (notSeen.find((*ii)->getVectorElementType()) != notSeen.end()) {
                        allSeen = false;
                    }
                }
                
                if (allSeen) {
                    changed = true;
                    types.push_back(*ii);
                    ii = notSeen.erase(ii);
                } else {
                    ii++;
                }
            }
        } while (changed);
    }
    
    void getCorrectFunctionOrder(list<AnnotatedFunction*> &funcs, AnnotatedModule* mod) {
        unordered_map<AnnotatedFunction*,TypeInfo> infos;
        for (list<AnnotatedFunction*>::iterator ii = mod->getFunctions()->begin(); ii != mod->getFunctions()->end(); ii++) {
            TypeInfo info;
            getTypeInfo(info, *ii);
            infos[*ii] = info;
        }
        
        unordered_set<AnnotatedFunction*> notSeen(mod->getFunctions()->begin(), mod->getFunctions()->end());
        
        // while there's still more nodes in need of calculation; that is, if the state changed since last iteration...
        bool changed;
        do {
            changed = false;
            
            for (unordered_set<AnnotatedFunction*>::iterator ii = notSeen.begin(); ii != notSeen.end(); /* ii++ is below */) {
                
                unordered_set<Function*>* called = &infos[*ii].funcsCalled;
                
                // if all dependancy nodes have been seen, then we are ready to add to the list.
                // Note that this means nodes with no dependencies automatically pass the test.
                bool allSeen = true;
                for (unordered_set<Function*>::iterator jj = called->begin(); jj != called->end(); jj++) {
                    AnnotatedFunction* func = mod->getFunction(*jj);
                    if (notSeen.find(func) != notSeen.end()) {
                        allSeen = false;
                        break;
                    }
                }
                
                if (allSeen) {
                    changed = true;
                    funcs.push_back(*ii);
                    ii = notSeen.erase(ii);
                } else {
                    ii++;
                }
            }
        } while (changed);
    }
    
    void addGoals(ostream &out, AnnotatedModule* module) {
        unsigned asserts = 1;
        unsigned calls = 1;
        
        for (Module::iterator ii = module->rawIR()->begin(); ii != module->rawIR()->end(); ii++) {
            AnnotatedFunction* func = module->getFunction(&*ii);
            if (func->getEnsuresClause()) {
                addGoal(out, func, "Goal_" + getWhy3SafeName(string(ii->getName().data())) + "_ensures", func->getEnsuresClause(), NULL);
            }
            
            for (Function::iterator jj = ii->begin(); jj != ii->end(); jj++) {
                for (BasicBlock::iterator kk = jj->begin(); kk != jj->end(); kk++) {
                    AnnotatedInstruction* inst = func->getAnnotatedInstruction(&*kk);
                    if (inst && inst->getAssertClause()) {
                        string goalName = "Goal_" + getWhy3SafeName(string(ii->getName().data())) + "_assert_";
                        if (inst->getAssertClause()->getSource() && inst->getAssertClause()->getSource()->label) {
                            goalName += inst->getAssertClause()->getSource()->label;
                        }
                        goalName += "_" + to_string(asserts);
                        addGoal(out, func, goalName, inst->getAssertClause(), &*kk);
                        asserts++;
                    }
                    
                    if (isa<CallInst>(&*kk)) {
                        CallInst* callInst = cast<CallInst>(&*kk);
                        Function* calledFuncRaw = callInst->getCalledFunction();
                        AnnotatedFunction* calledFunc = func->getModule()->getFunction(calledFuncRaw);
                        
                        if (calledFunc->getRequiresClause()) {
                            string goalName = "Goal_" + getWhy3SafeName(string(ii->getName().data())) + "_call_" + getWhy3SafeName(string(calledFuncRaw->getName().data())) + "_" + to_string(calls);
                            addGoal(out, func, goalName, calledFunc->getRequiresClause(), &*kk);
                            calls++;
                        }
                    }
                }
            }
        }
    }
    
    void generateWhy3(ostream &out, AnnotatedModule* module) {
        out << why3_header << endl;
        
        TypeInfo info;
        getTypeInfo(info, module);
        
        addMemoryModel(out, module);
        
        if (!info.ptrTypes.empty()) {
            info.intTypes.insert(Type::getIntNTy(module->rawIR()->getContext(), module->rawIR()->getDataLayout().getPointerSizeInBits(0))); // TODO: address spaces...
        }
        
        if (!info.vectorTypes.empty()) {
            for (unordered_set<VectorType*>::iterator ii = info.vectorTypes.begin(); ii != info.vectorTypes.end(); ii++) {
                if ((*ii)->getVectorElementType()->isPointerTy()) {
                    info.vectorTypes.insert(VectorType::get(Type::getIntNTy(module->rawIR()->getContext(), module->rawIR()->getDataLayout().getPointerSizeInBits(0)), (*ii)->getVectorNumElements())); // TODO: address spaces...
                }
            }
        }
        
        if (!info.intTypes.empty()) {
            addCommonIntType(out, module);
            
            for (unordered_set<IntegerType*>::iterator ii = info.intTypes.begin(); ii != info.intTypes.end(); ii++) {
                addIntType(out, module, *ii);
            }
        }
        
        if (!info.floatTypes.empty()) {
            addCommonFloatType(out, module);
            
            for (unordered_set<Type*>::iterator ii = info.floatTypes.begin(); ii != info.floatTypes.end(); ii++) {
                addFloatType(out, module, *ii);
            }
        }
        
        if (!info.ptrTypes.empty()) {
            addCommonPtrType(out, module);
        }
        
        if (!info.arrayTypes.empty()) {
            addCommonArrayType(out, module);
        }
        
        if (!info.structTypes.empty()) {
            addCommonStructType(out, module);
        }
        
        if (!info.vectorTypes.empty()) {
            addCommonVectorType(out, module);
        }
        
        list<Type*> types;
        getCorrectDerivedTypeOrder(types, info, module);
        
        for (list<Type*>::iterator ii = types.begin(); ii != types.end(); ii++) {
            addDerivedType(out, module, *ii);
        }
        
        if (info.usesBaddr) {
            addBlockAddressType(out, module);
        }
        
        addGlobals(out, module);
        
        list<AnnotatedFunction*> funcs;
        getCorrectFunctionOrder(funcs, module);
        
        for (list<AnnotatedFunction*>::iterator ii = funcs.begin(); ii != funcs.end(); ii++) {
            addFunction(out, *ii);
        }
        
        // TODO: recursive or mutually recursive functions are currently left out, because Why3 does not allow for out-of-order theory definitions.
        // In the future, we will have to add a handler for recursion (we can forward declare once inside a theory), and inline all mutually recursive functions.
        for (list<AnnotatedFunction*>::iterator ii = module->getFunctions()->begin(); ii != module->getFunctions()->end(); ii++) {
            if (find(funcs.begin(), funcs.end(), *ii) == funcs.end()) {
                // if the function was left out, warn the user
                if (module->getSettings()) {
                    module->getSettings()->warnings.push_back(whyr_warning("Function '" + string((*ii)->rawIR()->getName().data()) + "' is recursive or mutually recursive; function ignored"));
                }
            }
        }
        
        if (!module->getSettings() || (!module->getSettings()->noGoals && !module->getSettings()->combineGoals)) addGoals(out, module);
    }
}
