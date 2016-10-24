/*
 * war.cpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jrobbins
 */

#include <whyr/exception.hpp>
#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/war.hpp>

#include <sstream>

// YACC needs these defined.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef whyr::LogicExpression WarExpr;
typedef whyr::LogicLocal* WarDecl;
typedef std::list<WarDecl>* WarDeclList;
typedef std::pair<whyr::LogicLocal*, whyr::LogicExpression*>* WarDef;
typedef std::list<WarDef>* WarDefList;
typedef std::list<WarExpr*>* WarExprList;

extern "C" {
    int yywrap(void) {
        return 1;
    }
    
    int yylex();
    int yyparse();
    
    typedef union WarNode {
        char* token;
        WarExpr* expr;
        struct WarNodeQuant {
            bool kind;
            WarDeclList decls;
        } quant;
        WarDecl decl;
        WarDef def;
        WarDefList defs;
        WarExprList exprs;
    } WarNode;
}

#define YYSTYPE WarNode

/// this is the node source, used as a global argument for yyparse().
static whyr::NodeSource* warParserSource;

// This is our YACC parser return value. yyparse() will return its result in this field.
WarNode warParserReturnValue;

static void whyr_yyerror(char *s) {
    throw whyr::syntax_exception((std::string("in WAR expression: ") + s), NULL, warParserSource);
}

extern "C" void yyerror(char *s) {
    whyr_yyerror(s);
}

// ================================
// Below are all the functions used to parse our syntax tree.
// ================================

WarNode war_parse_int(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionIntegerConstant(node.token, warParserSource);
    return ret;
}

WarNode war_parse_bool_const(bool value) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionBooleanConstant(value, warParserSource);
    return ret;
}

WarNode war_parse_llvm_int_type(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    int bits = stoi(string(node.token));
    IntegerType* type = Type::getIntNTy(warParserSource->func->getModule()->rawIR()->getContext(), bits);
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_cast(WarNode typeNode, WarNode exprNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* castFrom = exprNode.expr->returnType();
    LogicType* castTo = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    
    // TODO: Handle other conversion cases
    if (castFrom->equals(castTo)) {
        // handle no-op conversions
        return exprNode;
    } else if (isa<LogicExpressionIntegerConstant>(exprNode.expr) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy()) {
        // handle integer constants -> LLVM int consts in an optimized way
        string value = cast<LogicExpressionIntegerConstant>(exprNode.expr)->getValue();
        IntegerType* intType = cast<IntegerType>(cast<LogicTypeLLVM>(castTo)->getType());
        ConstantInt* llvm = ConstantInt::get(intType, StringRef(value), 10);
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
        return ret;
    } else if (isa<LogicExpressionBooleanConstant>(exprNode.expr) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy()) {
        // handle boolean consts -> LLVM int consts in an optimized way
        bool value = cast<LogicExpressionBooleanConstant>(exprNode.expr)->getValue();
        IntegerType* intType = cast<IntegerType>(cast<LogicTypeLLVM>(castTo)->getType());
        ConstantInt* llvm = ConstantInt::get(intType, StringRef(value ? "1" : "0"), 10);
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
        return ret;
    } else if (isa<LogicExpressionRealConstant>(exprNode.expr) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isFloatTy()) {
        // handle real constants -> floats in an optimized way
        string value = cast<LogicExpressionRealConstant>(exprNode.expr)->getValue();
        Type* floatType = cast<LogicTypeLLVM>(castTo)->getType();
        APFloat flt(APFloat::IEEEsingle, StringRef(value));
        ConstantFP* llvm = ConstantFP::get(warParserSource->func->getModule()->rawIR()->getContext(), flt);
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
        return ret;
    } else if (isa<LogicExpressionRealConstant>(exprNode.expr) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isDoubleTy()) {
        // handle real constants -> doubles in an optimized way
        string value = cast<LogicExpressionRealConstant>(exprNode.expr)->getValue();
        Type* floatType = cast<LogicTypeLLVM>(castTo)->getType();
        APFloat flt(APFloat::IEEEdouble, StringRef(value));
        ConstantFP* llvm = ConstantFP::get(warParserSource->func->getModule()->rawIR()->getContext(), flt);
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isIntegerTy(1) && isa<LogicTypeBool>(castTo)) {
        // handle i1 -> boolean
        WarNode ret;
        ret.expr = new LogicExpressionBoolean(exprNode.expr, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isIntegerTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy() && cast<LogicTypeLLVM>(castFrom)->getType()->getIntegerBitWidth() > cast<LogicTypeLLVM>(castTo)->getType()->getIntegerBitWidth()) {
        // handle truncation of llvm ints
        WarNode ret;
        ret.expr = new LogicExpressionLLVMIntToLLVMInt(LogicExpressionLLVMIntToLLVMInt::OP_TRUNC, exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeInt>(castFrom) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy()) {
        // handle int -> llvm int
        WarNode ret;
        ret.expr = new LogicExpressionLogicIntToLLVMInt(exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isFloatingPointTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isFloatingPointTy()) {
        // handle float -> float
        WarNode ret;
        ret.expr = new LogicExpressionFloatToReal(exprNode.expr, warParserSource);
        ret.expr = new LogicExpressionRealToFloat(ret.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isFloatingPointTy() && isa<LogicTypeReal>(castTo)) {
        // handle float -> real
        WarNode ret;
        ret.expr = new LogicExpressionFloatToReal(exprNode.expr, warParserSource);
        return ret;
    } else if (isa<LogicTypeReal>(castFrom) && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isFloatingPointTy()) {
        // handle real -> float
        WarNode ret;
        ret.expr = new LogicExpressionRealToFloat(exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeReal>(castFrom) && isa<LogicTypeInt>(castTo)) {
        // handle real -> int
        WarNode ret;
        ret.expr = new LogicExpressionRealToInt(exprNode.expr, warParserSource);
        return ret;
    } else if (isa<LogicTypeInt>(castFrom) && isa<LogicTypeReal>(castTo)) {
        // handle int -> real
        WarNode ret;
        ret.expr = new LogicExpressionIntToReal(exprNode.expr, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isPointerTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy()) {
        // handle ptr -> int
        WarNode ret;
        ret.expr = new LogicExpressionPointerToInt(exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isIntegerTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isPointerTy()) {
        // handle int -> ptr
        WarNode ret;
        ret.expr = new LogicExpressionIntToPointer(exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isPointerTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isPointerTy()) {
        // handle ptr -> ptr
        WarNode ret;
        ret.expr = new LogicExpressionPointerToPointer(exprNode.expr, castTo, warParserSource);
        return ret;
    } else {
        throw type_exception(("Cannot convert an expression of type '"+castFrom->toString()+"' to type '"+castTo->toString()+"'"), NULL, warParserSource);
    }
}

WarNode war_parse_var(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    string varName = string(node.token+1);
    switch (node.token[0]) {
        case '%': {
            // either a local or an argument.
            if (warParserSource->inst) {
                WarNode ret;
                ret.expr = new LogicExpressionVariable(warParserSource->func->rawIR(), varName, warParserSource);
                return ret;
            } else {
                WarNode ret;
                ret.expr = new LogicExpressionArgument(warParserSource->func->rawIR(), varName, warParserSource);
                return ret;
            }
            break;
        }
        case '@': {
            // a global.
            WarNode ret;
            GlobalVariable* global = warParserSource->func->getModule()->rawIR()->getGlobalVariable(StringRef(varName));
            if (global) {
                ret.expr = new LogicExpressionLLVMConstant(global, warParserSource);
            } else {
                throw syntax_exception(("Unknown global variable '" + varName + "' in expression"), NULL, warParserSource);
            }
            return ret;
            break;
        }
        case '$': {
            // a logic-local.
            WarNode ret;
            ret.expr = new LogicExpressionLocal(varName, warParserSource);
            return ret;
            break;
        }
        default: {
            throw whyr_exception("internal error: Unknown variable type in WAR expression", NULL, warParserSource);
        }
    }
}

WarNode war_parse_var_ext(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    node.token = strdup(((*node.token) + string(node.token).substr(2, strlen(node.token)-3)).c_str());
    return war_parse_var(node);
}

WarNode war_parse_bin_bool_op(char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryBoolean::BinaryBooleanOp op;
    switch (opChar) {
        case 'i': {
            op = LogicExpressionBinaryBoolean::OP_IMPLIES; break;
        }
        case 'b': {
            op = LogicExpressionBinaryBoolean::OP_BIDIR_IMPLIES; break;
            break;
        }
        case '&': {
            op = LogicExpressionBinaryBoolean::OP_AND; break;
            break;
        }
        case '|': {
            op = LogicExpressionBinaryBoolean::OP_OR; break;
            break;
        }
        default: {
            throw whyr_exception("internal error: Unknown war_parse_bin_bool_op operator character", NULL, warParserSource);
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryBoolean(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_not(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionNot(node.expr);
    return ret;
}

WarNode war_parse_eq_op(bool negated, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionEquals(lhs.expr, rhs.expr, negated);
    return ret;
}

WarNode war_parse_bin_math_op(char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryMath::BinaryMathOp op;
    switch (opChar) {
        case '+': {
            op = LogicExpressionBinaryMath::OP_ADD; break;
        }
        case '-': {
            op = LogicExpressionBinaryMath::OP_SUB; break;
            break;
        }
        case '*': {
            op = LogicExpressionBinaryMath::OP_MUL; break;
            break;
        }
        case '/': {
            op = LogicExpressionBinaryMath::OP_DIV; break;
            break;
        }
        case '%': {
            op = LogicExpressionBinaryMath::OP_MOD; break;
            break;
        }
        case '$': {
            op = LogicExpressionBinaryMath::OP_REM; break;
            break;
        }
        default: {
            throw whyr_exception("internal error: Unknown war_parse_bin_math_op operator character", NULL, warParserSource);
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryMath(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_int_type() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeInt(warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_bool_type() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeBool(warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_decl_item(WarNode typeNode, WarNode varNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.decl = new LogicLocal();
    if (varNode.token[0] != '$') {
        throw syntax_exception(("Local declaration '" + string(varNode.token) + "' needs to be a local variable"), NULL, warParserSource);
    }
    ret.decl->name = string(varNode.token+1);
    ret.decl->type = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    return ret;
}

WarNode war_parse_decl_item_ext(WarNode typeNode, WarNode varNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    varNode.token = strdup(((*varNode.token) + string(varNode.token).substr(2, strlen(varNode.token)-3)).c_str());
    return war_parse_decl_item(typeNode, varNode);
}

WarNode war_init_decl_list(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.quant.decls = new list<LogicLocal*>();
    ret.quant.decls->push_back(node.decl);
    return ret;
}

WarNode war_extend_decl_list(WarNode listNode, WarNode itemNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    listNode.quant.decls->push_back(itemNode.decl);
    return listNode;
}

WarNode war_add_quant_locals(WarNode kindNode, WarNode listNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    for (list<LogicLocal*>::iterator ii = listNode.quant.decls->begin(); ii != listNode.quant.decls->end(); ii++) {
        warParserSource->logicLocals[(*ii)->name].push_back(*ii);
    }
    
    WarNode node;
    node.quant.kind = kindNode.quant.kind;
    node.quant.decls = listNode.quant.decls;
    return node;
}

WarNode war_parse_quant(WarNode quantNode, WarNode exprNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    for (list<LogicLocal*>::iterator ii = quantNode.quant.decls->begin(); ii != quantNode.quant.decls->end(); ii++) {
        warParserSource->logicLocals[(*ii)->name].pop_back();
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionQuantifier(quantNode.quant.kind, quantNode.quant.decls, exprNode.expr, warParserSource);
    return ret;
}

WarNode war_parse_neg(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionNegate(node.expr, warParserSource);
    return ret;
}

WarNode war_parse_ifte(WarNode& condNode, WarNode& trueNode, WarNode& falseNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionConditional(condNode.expr, trueNode.expr, falseNode.expr, warParserSource);
    return ret;
}

WarNode war_parse_def_item(WarNode declNode, WarNode valueNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.def = new pair<LogicLocal*, LogicExpression*>(declNode.decl, valueNode.expr);
    return ret;
}

WarNode war_init_def_list(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.defs = new list<WarDef>();
    ret.defs->push_back(node.def);
    return ret;
}

WarNode war_extend_def_list(WarNode listNode, WarNode itemNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    listNode.defs->push_back(itemNode.def);
    return listNode;
}

WarNode war_add_let_locals(WarNode listNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    for (list<WarDef>::iterator ii = listNode.defs->begin(); ii != listNode.defs->end(); ii++) {
        warParserSource->logicLocals[(*ii)->first->name].push_back((*ii)->first);
    }
    
    return listNode;
}

WarNode war_parse_let(WarNode listNode, WarNode exprNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    for (list<WarDef>::iterator ii = listNode.defs->begin(); ii != listNode.defs->end(); ii++) {
        warParserSource->logicLocals[(*ii)->first->name].pop_back();
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionLet(listNode.defs, exprNode.expr, warParserSource);
    return ret;
}

WarNode war_parse_result() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    if (warParserSource->inst != NULL) {
        throw syntax_exception("'result' meaningless outside of function contract", NULL, warParserSource);
    }
    
    if (warParserSource->func->rawIR()->getReturnType()->isVoidTy()) {
        throw syntax_exception("Use of 'result' in function returning void", NULL, warParserSource);
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionResult(new LogicTypeLLVM(warParserSource->func->rawIR()->getReturnType(), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_bin_bits_op(char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryBits::BinaryBitsOp op;
    switch (opChar) {
        case '&': {
            op = LogicExpressionBinaryBits::OP_AND; break;
        }
        case '|': {
            op = LogicExpressionBinaryBits::OP_OR; break;
        }
        case '^': {
            op = LogicExpressionBinaryBits::OP_XOR; break;
        }
        case '/': {
            op = LogicExpressionBinaryBits::OP_SDIV; break;
        }
        case '%': {
            op = LogicExpressionBinaryBits::OP_SMOD; break;
        }
        case '$': {
            op = LogicExpressionBinaryBits::OP_SREM; break;
        }
        case '?': {
            op = LogicExpressionBinaryBits::OP_UDIV; break;
        }
        case '5': {
            op = LogicExpressionBinaryBits::OP_UMOD; break;
        }
        case '4': {
            op = LogicExpressionBinaryBits::OP_UREM; break;
        }
        default: {
            throw whyr_exception("internal error: Unknown war_parse_bin_bits_op operator character", NULL, warParserSource);
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryBits(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_bin_comp_op(char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryCompare::BinaryCompareOp op;
    switch (opChar) {
        case '<': {
            op = LogicExpressionBinaryCompare::OP_LT; break;
        }
        case '>': {
            op = LogicExpressionBinaryCompare::OP_GT; break;
        }
        case '.': {
            op = LogicExpressionBinaryCompare::OP_GE; break;
        }
        case ',': {
            op = LogicExpressionBinaryCompare::OP_LE; break;
        }
        default: {
            throw whyr_exception("internal error: Unknown war_parse_bin_comp_op operator character", NULL, warParserSource);
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryCompare(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_bin_comp_llvm_op(bool sign, char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryCompareLLVM::BinaryCompareLLVMOp op;
    if (sign) {
        switch (opChar) {
            case '<': {
                op = LogicExpressionBinaryCompareLLVM::OP_SLT; break;
            }
            case '>': {
                op = LogicExpressionBinaryCompareLLVM::OP_SGT; break;
            }
            case '.': {
                op = LogicExpressionBinaryCompareLLVM::OP_SGE; break;
            }
            case ',': {
                op = LogicExpressionBinaryCompareLLVM::OP_SLE; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_bin_comp_llvm_op operator character", NULL, warParserSource);
            }
        }
    } else {
        switch (opChar) {
            case '<': {
                op = LogicExpressionBinaryCompareLLVM::OP_ULT; break;
            }
            case '>': {
                op = LogicExpressionBinaryCompareLLVM::OP_UGT; break;
            }
            case '.': {
                op = LogicExpressionBinaryCompareLLVM::OP_UGE; break;
            }
            case ',': {
                op = LogicExpressionBinaryCompareLLVM::OP_ULE; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_bin_comp_llvm_op operator character", NULL, warParserSource);
            }
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryCompareLLVM(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_shift_op(char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryShift::BinaryShiftOp op;
    switch (opChar) {
        case '<': {
            op = LogicExpressionBinaryShift::OP_LSHL; break;
        }
        case '>': {
            op = LogicExpressionBinaryShift::OP_LSHR; break;
        }
        case '?': {
            op = LogicExpressionBinaryShift::OP_ASHR; break;
        }
        default: {
            throw whyr_exception("internal error: Unknown war_parse_shift_op operator character", NULL, warParserSource);
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryShift(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_bit_not(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionBitNot(node.expr);
    return ret;
}

WarNode war_parse_cast_ext(bool isZext, WarNode typeNode, WarNode exprNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* castFrom = exprNode.expr->returnType();
    LogicType* castTo = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    
    // TODO: Handle other conversion cases
    if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isIntegerTy() && isa<LogicTypeLLVM>(castTo) && cast<LogicTypeLLVM>(castTo)->getType()->isIntegerTy() && cast<LogicTypeLLVM>(castFrom)->getType()->getIntegerBitWidth() < cast<LogicTypeLLVM>(castTo)->getType()->getIntegerBitWidth()) {
        // handle extension of llvm ints
        LogicExpressionLLVMIntToLLVMInt::LLVMIntToLLVMIntOp op;
        if (isZext) {
            op = LogicExpressionLLVMIntToLLVMInt::OP_ZEXT;
        } else {
            op = LogicExpressionLLVMIntToLLVMInt::OP_SEXT;
        }
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMIntToLLVMInt(op, exprNode.expr, castTo, warParserSource);
        return ret;
    } else if (isa<LogicTypeLLVM>(castFrom) && cast<LogicTypeLLVM>(castFrom)->getType()->isIntegerTy() && isa<LogicTypeInt>(castTo)) {
        // handle llvm int -> logic int
        LogicExpressionLLVMIntToLogicInt::LLVMIntToLogicIntOp op;
        if (isZext) {
            op = LogicExpressionLLVMIntToLogicInt::OP_ZEXT;
        } else {
            op = LogicExpressionLLVMIntToLogicInt::OP_SEXT;
        }
        
        WarNode ret;
        ret.expr = new LogicExpressionLLVMIntToLogicInt(op, exprNode.expr, warParserSource);
        return ret;
    } else {
        throw type_exception(("Cannot convert an expression of type '"+castFrom->toString()+"' to type '"+castTo->toString()+"'"), NULL, warParserSource);
    }
}

WarNode war_parse_real(WarNode& node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionRealConstant(node.token, warParserSource);
    return ret;
}

WarNode war_parse_real_type() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeReal(warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_llvm_float_type() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    Type* type = Type::getFloatTy(warParserSource->func->getModule()->rawIR()->getContext());
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_llvm_double_type() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    Type* type = Type::getDoubleTy(warParserSource->func->getModule()->rawIR()->getContext());
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_bin_comp_float_op(bool ordered, char opChar, WarNode& lhs, WarNode& rhs) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionBinaryCompareFloat::BinaryCompareFloatOp op;
    if (ordered) {
        switch (opChar) {
            case '=': {
                op = LogicExpressionBinaryCompareFloat::OP_OEQ; break;
            }
            case '<': {
                op = LogicExpressionBinaryCompareFloat::OP_OLT; break;
            }
            case '>': {
                op = LogicExpressionBinaryCompareFloat::OP_OGT; break;
            }
            case '.': {
                op = LogicExpressionBinaryCompareFloat::OP_OGE; break;
            }
            case ',': {
                op = LogicExpressionBinaryCompareFloat::OP_OLE; break;
            }
            case '+': {
                op = LogicExpressionBinaryCompareFloat::OP_ONE; break;
            }
            case '!': {
                op = LogicExpressionBinaryCompareFloat::OP_ORD; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_bin_comp_float_op operator character", NULL, warParserSource);
            }
        }
    } else {
        switch (opChar) {
            case '=': {
                op = LogicExpressionBinaryCompareFloat::OP_UEQ; break;
            }
            case '<': {
                op = LogicExpressionBinaryCompareFloat::OP_ULT; break;
            }
            case '>': {
                op = LogicExpressionBinaryCompareFloat::OP_UGT; break;
            }
            case '.': {
                op = LogicExpressionBinaryCompareFloat::OP_UGE; break;
            }
            case ',': {
                op = LogicExpressionBinaryCompareFloat::OP_ULE; break;
            }
            case '+': {
                op = LogicExpressionBinaryCompareFloat::OP_UNE; break;
            }
            case '!': {
                op = LogicExpressionBinaryCompareFloat::OP_UNO; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_bin_comp_float_op operator character", NULL, warParserSource);
            }
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBinaryCompareFloat(op, lhs.expr, rhs.expr);
    return ret;
}

WarNode war_parse_llvm_ptr_type(WarNode typeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* logicType = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    if (!isa<LogicTypeLLVM>(logicType)) {
        throw type_exception("Pointer types must point to LLVM types; got a pointer of type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    Type* baseType = cast<LogicTypeLLVM>(logicType)->getType();
    PointerType* type = PointerType::get(baseType, 0); // TODO: address spaces...
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_cast_null(WarNode typeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* logicType = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    if (!isa<LogicTypeLLVM>(logicType)) {
        throw type_exception("null must be of pointer type; got type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    Type* type = cast<LogicTypeLLVM>(logicType)->getType();
    if (!type->isPointerTy()) {
        throw type_exception("null must be of pointer type; got type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    PointerType* ptrType = cast<PointerType>(type);
    ConstantPointerNull* llvm = ConstantPointerNull::get(ptrType);
    
    WarNode ret;
    ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
    return ret;
}

WarNode war_parse_load(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionLoad(node.expr, warParserSource);
    return ret;
}

WarNode war_parse_index(WarNode arrayNode, WarNode indexNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionGetIndex(arrayNode.expr, indexNode.expr, warParserSource);
    return ret;
}

WarNode war_init_array_item(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.exprs = new list<WarExpr*>();
    ret.exprs->push_back(node.expr);
    return ret;
}

WarNode war_extend_array_item(WarNode listNode, WarNode itemNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    listNode.exprs->push_back(itemNode.expr);
    return listNode;
}

WarNode war_parse_array_const(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* baseTypeRaw = node.exprs->front()->returnType();
    if (!isa<LogicTypeLLVM>(baseTypeRaw)) {
        throw type_exception("Elements of array constant must be an LLVM type, got type '" + baseTypeRaw->toString() + "'", NULL, warParserSource);
    }
    LogicTypeLLVM* baseType = cast<LogicTypeLLVM>(baseTypeRaw);
    
    WarNode ret;
    ret.expr = new LogicExpressionLLVMArrayConstant(baseType->getType(), node.exprs, warParserSource);
    return ret;
}

WarNode war_parse_llvm_array_type(WarNode typeNode, WarNode sizeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* logicType = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    if (!isa<LogicTypeLLVM>(logicType)) {
        throw type_exception("Array types must be made of LLVM types; got an array of type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    Type* baseType = cast<LogicTypeLLVM>(logicType)->getType();
    
    uint64_t elems = strtoull(sizeNode.token, NULL, 0);
    
    ArrayType* type = ArrayType::get(baseType, elems);
    
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_llvm_array_0_type(WarNode typeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode sizeNode;
    sizeNode.token = (char*) "0";
    return war_parse_llvm_array_type(typeNode, sizeNode);
}

WarNode war_parse_cast_empty_array(WarNode typeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* logicType = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    if (!isa<LogicTypeLLVM>(logicType)) {
        throw type_exception("Cannot cast expression '{}' to non-aggregate type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    Type* arrType = cast<LogicTypeLLVM>(logicType)->getType();
    if (!isa<ArrayType>(arrType)) {
        throw type_exception("Cannot cast expression '{}' to non-aggregate type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    if (arrType->getArrayNumElements() != 0) {
        throw type_exception("Cannot cast expression '{}' to type '" + logicType->toString() + "'; type has non-zero array length", NULL, warParserSource);
    }
    Constant* array = ConstantArray::get(cast<ArrayType>(arrType), ArrayRef<Constant*>(NoneType::None));
    
    WarNode ret;
    ret.expr = new LogicExpressionLLVMConstant(array, warParserSource);
    return ret;
}

WarNode war_parse_set_index(WarNode arrayNode, WarNode indexNode, WarNode valueNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    WarNode ret;
    ret.expr = new LogicExpressionSetIndex(arrayNode.expr, indexNode.expr, valueNode.expr, warParserSource);
    return ret;
}

WarNode war_parse_ref(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    list<LogicExpression*>* indices = new list<LogicExpression*>();
    // unroll array accesses into the index list
    
    while (isa<LogicExpressionGetIndex>(node.expr)) {
        indices->push_front(cast<LogicExpressionGetIndex>(node.expr)->getRight());
        node.expr = cast<LogicExpressionGetIndex>(node.expr)->getLeft();
    }
    
    // the remaining expr is the pointer
    WarNode ret;
    ret.expr = new LogicExpressionGetElementPointer(node.expr, indices, warParserSource);
    return ret;
}

WarNode war_parse_spec_llvm_const(bool isZext, char opChar, WarNode typeNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicExpressionSpecialLLVMConstant::SpecialLLVMConstOp op;
    if (isZext) {
        switch (opChar) {
            case '<': {
                op = LogicExpressionSpecialLLVMConstant::OP_MINUINT; break;
            }
            case '>': {
                op = LogicExpressionSpecialLLVMConstant::OP_MAXUINT; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_spec_llvm_const operator character", NULL, warParserSource);
            }
        }
    } else {
        switch (opChar) {
            case '<': {
                op = LogicExpressionSpecialLLVMConstant::OP_MININT; break;
            }
            case '>': {
                op = LogicExpressionSpecialLLVMConstant::OP_MAXINT; break;
            }
            default: {
                throw whyr_exception("internal error: Unknown war_parse_spec_llvm_const operator character", NULL, warParserSource);
            }
        }
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionSpecialLLVMConstant(op, cast<LogicTypeType>(typeNode.expr->returnType())->getType(), warParserSource);
    return ret;
}

WarNode war_parse_baddr(WarNode funcNode, WarNode blockNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    // funcNode has to be a pointer to a function.
    if (!isa<LogicExpressionLLVMConstant>(funcNode.expr)) {
        throw syntax_exception("Argument 1 to 'blockaddress' has to be a constant pointer to a function", NULL, warParserSource);
    }
    Constant* c = cast<LogicExpressionLLVMConstant>(funcNode.expr)->getValue();
    Function* func = warParserSource->func->getModule()->rawIR()->getFunction(c->getName());
    if (!func) {
        throw syntax_exception("Argument 1 to 'blockaddress' has to be a constant pointer to a function", NULL, warParserSource);
    }
    
    // blockNode needs to have the correct name
    if (*blockNode.token != '%') {
        throw syntax_exception("Argument 2 to 'blockaddress' has to be a label name", NULL, warParserSource);
    }
    BasicBlock* block = NULL;
    for (Function::iterator ii = func->begin(); ii != func->end(); ii++) {
        if (ii->getName().compare(StringRef(blockNode.token+1)) == 0) {
            block = &*ii;
            break;
        }
    }
    if (!block) {
        throw syntax_exception("Argument 2 to 'blockaddress' has to be a label name", NULL, warParserSource);
    }
    
    WarNode ret;
    ret.expr = new LogicExpressionBlockAddress(func, block, warParserSource);
    return ret;
}

WarNode war_parse_baddr_ext(WarNode funcNode, WarNode blockNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    blockNode.token = strdup(((*blockNode.token) + string(blockNode.token).substr(2, strlen(blockNode.token)-3)).c_str());
    return war_parse_baddr(funcNode, blockNode);
}

WarNode war_parse_struct_type(WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    // find the type it corresponds to
    StructType* type = warParserSource->func->getModule()->rawIR()->getTypeByName(StringRef(node.token));
    if (!type) {
        throw syntax_exception("Type '%" + string(node.token) + "' could not be found", NULL, warParserSource);
    }
    
    // return struct type
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_anon_struct_type(bool packed, WarNode node) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    Type* types[node.exprs->size()]; int i = 0;
    for (list<LogicExpression*>::iterator ii = node.exprs->begin(); ii != node.exprs->end(); ii++) {
        LogicType* logicType = cast<LogicTypeType>((*ii)->returnType())->getType();
        if (!isa<LogicTypeLLVM>(logicType)) {
            throw type_exception("Struct types must be made of LLVM types; got a member of type '" + logicType->toString() + "'", NULL, warParserSource);
        }
        Type* baseType = cast<LogicTypeLLVM>(logicType)->getType();
        types[i] = baseType;
        
        i++;
    }
    
    StructType* type = StructType::get(warParserSource->func->rawIR()->getContext(), ArrayRef<Type*>(types, node.exprs->size()), packed);
    WarNode ret;
    ret.expr = new LogicExpressionConstantType(new LogicTypeType(new LogicTypeLLVM(type, warParserSource), warParserSource), warParserSource);
    return ret;
}

WarNode war_parse_struct_const(WarNode typeNode, WarNode listNode) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    LogicType* logicType = cast<LogicTypeType>(typeNode.expr->returnType())->getType();
    if (!isa<LogicTypeLLVM>(logicType)) {
        throw type_exception("Struct constants must be cast to struct type; got type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    
    Type* type = cast<LogicTypeLLVM>(logicType)->getType();
    if (!type->isStructTy()) {
        throw type_exception("Struct constants must be cast to struct type; got type '" + logicType->toString() + "'", NULL, warParserSource);
    }
    
    if (listNode.exprs->size() != type->getStructNumElements()) {
        throw type_exception("Struct type '" + logicType->toString() + "' has " + to_string(type->getStructNumElements()) + " elements; got " + to_string(listNode.exprs->size()), NULL, warParserSource);
    }
    
    Constant* items[listNode.exprs->size()];
    int i = 0;
    for (list<LogicExpression*>::iterator ii = listNode.exprs->begin(); ii != listNode.exprs->end(); ii++) {
        if (!isa<LogicExpressionLLVMConstant>(*ii)) {
            throw type_exception("Expected item " + to_string(i) + " to be of type '" + LogicTypeLLVM(type->getStructElementType(i)).toString() + "'; got type '" + (*ii)->returnType()->toString() + "'", NULL, warParserSource);
        }
        items[i] = cast<LogicExpressionLLVMConstant>(*ii)->getValue();
        if (items[i]->getType() != type->getStructElementType(i)) {
            throw type_exception("Expected item " + to_string(i) + " to be of type '" + LogicTypeLLVM(type->getStructElementType(i)).toString() + "'; got type '" + (*ii)->returnType()->toString() + "'", NULL, warParserSource);
        }
        i++;
    }
    
    StructType* structType = cast<StructType>(type);
    Constant* llvm = ConstantStruct::get(structType, ArrayRef<Constant*>(items, listNode.exprs->size()));
    
    WarNode ret;
    ret.expr = new LogicExpressionLLVMConstant(llvm, warParserSource);
    return ret;
}

// ================================
// End of parser functions.
// ================================

// import our YACC parser.
#include "war_parser.hpp"
#include "war_lexer.hpp"

// our actual WAR API.
namespace whyr {
    using namespace std;
    using namespace llvm;
    
    LogicExpression* parseWarString(string war, NodeSource* source) {
        char* buffer = strdup(war.c_str());
        yyin = fmemopen(buffer, war.size(), "r");
        
        warParserSource = new NodeSource(source);
        yyparse();
        
        fclose(yyin);
        free(buffer);
        
        return warParserReturnValue.expr;
    }
}
