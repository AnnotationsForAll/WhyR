/*
 * rte.cpp
 *
 *  Created on: Oct 13, 2016
 *      Author: jrobbins
 */

#include <whyr/rte.hpp>

#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>

#include <llvm/IR/Operator.h>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static LogicExpression* getBinMathOverflowExpr(Instruction* inst, LogicExpressionBinaryMath::BinaryMathOp op) {
        LogicExpression* expr = NULL;
        LogicExpression* nuw = NULL;
        LogicExpression* nsw = NULL;
        
        // for "a = add nuw b,c", add the assertion that "b + c uge min && b + c ule max"
        if (cast<BinaryOperator>(inst)->hasNoUnsignedWrap()) {
            expr = nuw = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                    new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_UGE,
                            new LogicExpressionBinaryMath(op,
                                    new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                    new LogicExpressionLLVMOperand(inst->getOperand(1))
                            ),
                            new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MINUINT, new LogicTypeLLVM(inst->getType()))
                    ),
                    new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_ULE,
                            new LogicExpressionBinaryMath(op,
                                    new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                    new LogicExpressionLLVMOperand(inst->getOperand(1))
                            ),
                            new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MAXUINT, new LogicTypeLLVM(inst->getType()))
                    )
            );
        }
        
        // for "a = add nsw b,c", add the assertion that "b + c sge min && b + c sle max"
        if (cast<BinaryOperator>(inst)->hasNoSignedWrap()) {
            expr = nsw = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                    new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_SGE,
                            new LogicExpressionBinaryMath(op,
                                    new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                    new LogicExpressionLLVMOperand(inst->getOperand(1))
                            ),
                            new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MININT, new LogicTypeLLVM(inst->getType()))
                    ),
                    new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_SLE,
                            new LogicExpressionBinaryMath(op,
                                    new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                    new LogicExpressionLLVMOperand(inst->getOperand(1))
                            ),
                            new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MAXINT, new LogicTypeLLVM(inst->getType()))
                    )
            );
        }
        
        // for "a = add nuw nsw b,c", add the AND of the above assertions
        if (nsw && nuw) {
            expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND, nsw, nuw);
        }
        
        return expr;
    }
    
    void addRTE(AnnotatedFunction* func, Instruction* inst) {
        LogicExpression* expr = NULL;
        
        // if the instruction needs RTE, put in expr what the assertion is; else, return
        switch (inst->getOpcode()) {
            case Instruction::BinaryOps::Add: {
                expr = getBinMathOverflowExpr(inst, LogicExpressionBinaryMath::OP_ADD);
                break;
            }
            case Instruction::BinaryOps::Sub: {
                expr = getBinMathOverflowExpr(inst, LogicExpressionBinaryMath::OP_SUB);
                break;
            }
            case Instruction::BinaryOps::Mul: {
                expr = getBinMathOverflowExpr(inst, LogicExpressionBinaryMath::OP_MUL);
                break;
            }
            case Instruction::BinaryOps::SDiv:
            case Instruction::BinaryOps::SRem: {
                // for "a = sdiv b, c", add the assertion that "b <> min || c <> -1"
                expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_OR,
                        new LogicExpressionEquals(
                                new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MININT, new LogicTypeLLVM(inst->getType()))
                        , true),
                        new LogicExpressionEquals(
                                new LogicExpressionLLVMOperand(inst->getOperand(1)),
                                new LogicExpressionLLVMConstant(ConstantInt::get(inst->getType(), -1, true))
                        , true)
                );
            }
            case Instruction::BinaryOps::UDiv:
            case Instruction::BinaryOps::URem:
            {
                // for "a = div b, c", add the assertion that "c <> 0"
                
                LogicExpression* dneExpr = new LogicExpressionEquals(
                        new LogicExpressionLLVMOperand(inst->getOperand(1)),
                        new LogicExpressionLLVMConstant(ConstantInt::get(cast<IntegerType>(inst->getType()), 0, false))
                , true);
                
                // chain the assertion with the SDiv above, if applicable
                if (expr) {
                    expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                            dneExpr,
                            expr
                    );
                } else {
                    expr = dneExpr;
                }
                
                // for "a = div exact b, c", add the assertion that "c <> 0 && (int sext)b mod (int sext)c == 0"
                if (isa<PossiblyExactOperator>(inst) && cast<PossiblyExactOperator>(inst)->isExact()) {
                    expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                            expr,
                            new LogicExpressionEquals(
                                    new LogicExpressionBinaryMath(LogicExpressionBinaryMath::OP_MOD,
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_SEXT,
                                                    new LogicExpressionLLVMOperand(inst->getOperand(0))
                                            ),
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_SEXT,
                                                    new LogicExpressionLLVMOperand(inst->getOperand(1))
                                            )
                                    ),
                                    new LogicExpressionIntegerConstant("0")
                            )
                    );
                }
                break;
            }
            case Instruction::BinaryOps::Shl: {
                LogicExpression* nuw = NULL;
                LogicExpression* nsw = NULL;
                
                // for "a = shl nuw b, c", add the assertion that "(b shl (int zext)c) ule max"
                if (cast<BinaryOperator>(inst)->hasNoUnsignedWrap()) {
                    expr = nuw = new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_ULE,
                                    new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_LSHL,
                                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                        new LogicExpressionLLVMOperand(inst->getOperand(1))
                                            )
                                    ),
                                    new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MAXUINT, new LogicTypeLLVM(inst->getType()))
                            );
                }
                
                // for "a = shl nsw b, c", add the assertion that "(b shl (int zext)c) sle max"
                if (cast<BinaryOperator>(inst)->hasNoSignedWrap()) {
                    expr = nuw = new LogicExpressionBinaryCompareLLVM(LogicExpressionBinaryCompareLLVM::OP_SLE,
                                    new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_LSHL,
                                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                        new LogicExpressionLLVMOperand(inst->getOperand(1))
                                            )
                                    ),
                                    new LogicExpressionSpecialLLVMConstant(LogicExpressionSpecialLLVMConstant::OP_MAXINT, new LogicTypeLLVM(inst->getType()))
                            );
                }
                
                // for "a = shl nuw nsw b, c", AND the two together
                if (nsw && nuw) {
                    expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND, nsw, nuw);
                }
                
                break;
            }
            case Instruction::BinaryOps::LShr: {
                // for "a = lshr exact b, c", add the assertion that "b == ((b lshr (int zext)c) shl (int zext)c)"
                if (isa<PossiblyExactOperator>(inst) && cast<PossiblyExactOperator>(inst)->isExact()) {
                    expr = new LogicExpressionEquals(
                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                            new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_LSHL,
                                    new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_LSHR,
                                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                        new LogicExpressionLLVMOperand(inst->getOperand(1))
                                            )
                                    ),
                                    new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                new LogicExpressionLLVMOperand(inst->getOperand(1))
                                    )
                            )
                    ,false);
                }
                break;
            }
            case Instruction::BinaryOps::AShr: {
                // for "a = ashr exact b, c", add the assertion that "b == ((b ashr (int zext)c) shl (int zext)c)"
                if (isa<PossiblyExactOperator>(inst) && cast<PossiblyExactOperator>(inst)->isExact()) {
                    expr = new LogicExpressionEquals(
                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                            new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_LSHL,
                                    new LogicExpressionBinaryShift(LogicExpressionBinaryShift::OP_ASHR,
                                            new LogicExpressionLLVMOperand(inst->getOperand(0)),
                                            new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                        new LogicExpressionLLVMOperand(inst->getOperand(1))
                                            )
                                    ),
                                    new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT,
                                                new LogicExpressionLLVMOperand(inst->getOperand(1))
                                    )
                            )
                    ,false);
                }
                break;
            }
            default: {
                // nothing needs to be done
                return;
            }
        }
        
        if (expr) {
            expr->checkTypes();
        
            AnnotatedInstruction* annInst = func->getAnnotatedInstruction(inst);
            if (annInst) {
                if (annInst->getAssertClause()) {
                    // If the instruction already has an assert clause, AND them together
                    expr = new LogicExpressionBinaryBoolean(LogicExpressionBinaryBoolean::OP_AND,
                            annInst->getAssertClause(),
                            expr,
                    annInst->getAssertClause()->getSource());
                }
                annInst->setAssertClause(expr);
            } else {
                annInst = new AnnotatedInstruction(func, inst);
                annInst->setAssertClause(expr);
                func->getAnnotatedInstructions()->push_back(annInst);
            }
        }
    }
    
    void addRTE(AnnotatedFunction* func) {
        for (Function::iterator ii = func->rawIR()->begin(); ii != func->rawIR()->end(); ii++) {
            for (BasicBlock::iterator jj = ii->begin(); jj != ii->end(); jj++) {
                addRTE(func, &*jj);
            }
        }
    }
    
    void addRTE(AnnotatedModule* module) {
        for (list<AnnotatedFunction*>::iterator ii = module->getFunctions()->begin(); ii != module->getFunctions()->end(); ii++) {
            addRTE(*ii);
        }
    }
}
