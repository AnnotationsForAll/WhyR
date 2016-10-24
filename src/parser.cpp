/*
 * parser.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/exception.hpp>
#include <whyr/war.hpp>

#include <cstring>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    void ExpressionParser::requireMinArgs(MDNode* &node, const char* &exprName, NodeSource* source, unsigned nArgs) {
        if (node->getNumOperands() - 1 < nArgs) {
            throw syntax_exception("Expected " + to_string(nArgs) + " arguments to expression '" + exprName + "'; got " + to_string(node->getNumOperands() - 1), NULL, source);
        }
    }
    
    void ExpressionParser::requireMaxArgs(MDNode* &node, const char* &exprName, NodeSource* source, unsigned nArgs) {
        if (node->getNumOperands() - 1 > nArgs) {
            throw syntax_exception("Expected " + to_string(nArgs) + " arguments to expression '" + exprName + "'; got " + to_string(node->getNumOperands() - 1), NULL, source);
        }
    }
    
    LogicExpression* ExpressionParser::parseMetadata(Metadata* node, NodeSource* source) {
        // All expressions are either LLVM constants, or nodes representing named expression parsers.
        // It's like s-expressions, except with metadata nodes.
        if (isa<ConstantAsMetadata>(node)) {
            ConstantAsMetadata* nn = cast<ConstantAsMetadata>(node);
            return new LogicExpressionLLVMConstant(nn->getValue(), source);
        } else if (isa<MDNode>(node)) {
            MDNode* nn = cast<MDNode>(node);
            
            if (nn->getNumOperands() <= 0) {
                throw syntax_exception("Blank metadata nodes illegal in WhyR expressions", NULL, source);
            }
            
            Metadata* exprNameOperand = nn->getOperand(0).get();
            const char* exprName = NULL;
            if (isa<MDNode>(exprNameOperand)) {
                // this contains the command as well as debug location info
                source = new NodeSource(source);
                source->debugInfo.clear();
                MDNode* infoNode = cast<MDNode>(exprNameOperand);
                if (infoNode->getNumOperands() == 0) {
                    throw syntax_exception("Blank command with debug info nodes illegal in WhyR expressions", NULL, source);
                }
                if (isa<MDString>(infoNode->getOperand(0).get())) {
                    exprName = cast<MDString>(infoNode->getOperand(0).get())->getString().data();
                } else {
                    throw syntax_exception("First operand of command node with debug info needs to be a metadata string", NULL, source);
                }
                for (unsigned i = 1; i < infoNode->getNumOperands(); i++) {
                    if (isa<MDNode>(infoNode->getOperand(i).get())) {
                        MDNode* debugNode = cast<MDNode>(infoNode->getOperand(i).get());
                        LogicDebugInfo info;
                        
                        for (unsigned j = 0; j < debugNode->getNumOperands(); j++) {
                            if (isa<MDString>(debugNode->getOperand(j).get())) {
                                const char* item = cast<MDString>(debugNode->getOperand(j).get())->getString().data();
                                
                                switch (j) {
                                    case 0: {
                                        info.file = item;
                                        break;
                                    }
                                    case 1: {
                                        info.line = item;
                                        break;
                                    }
                                    case 2: {
                                        info.col1 = item;
                                        break;
                                    }
                                    case 3: {
                                        info.col2 = item;
                                        break;
                                    }
                                }
                            } else {
                                throw syntax_exception("All operands of debug info node needs to be a metadata string", NULL, source);
                            }
                        }
                        
                        source->debugInfo.push_back(info);
                    } else if (isa<MDString>(infoNode->getOperand(i).get())) {
                        source->label = cast<MDString>(infoNode->getOperand(i).get())->getString().data();
                    } else {
                        throw syntax_exception("Debug info nodes need to be metadata nodes or strings", NULL, source);
                    }
                }
            } else if (isa<MDString>(exprNameOperand)) {
                // this just contains the command name
                MDString* exprNameNode = cast<MDString>(exprNameOperand);
                exprName = exprNameNode->getString().data();
            } else {
                throw syntax_exception("First operand of expression node needs to be a metadata string (or metadata node with debug info)", NULL, source);
            }
            
            // Get the parser corresponding to the node's first argument, and execute it
            map<string, ExpressionParser*>::iterator found = getExpressionParsers()->find(exprName);
            if (found != getExpressionParsers()->end()) {
                ExpressionParser* parser = found->second;
                return parser->parse(exprName, nn, source);
            } else {
                throw syntax_exception("Unrecognized expression type '" + string(exprName) + "'", NULL, source);
            }
        } else {
            throw syntax_exception("an expression needs to be metadata node or constant value", NULL, source);
        }
    }
    
    ExpressionParser::ExpressionParser() {}
    ExpressionParser::~ExpressionParser() {}
    
    /* =======================
     * PARSER REGISTRY CLASSES
     * =======================
     * 
     * Add parsers for new expression types here.
     * 
     * This is separate from the expression logic itself, because parsers may not map 1-1 to expressions.
     * For example, ParserTrueFalse represents the names "true" and "false", and both map to a LogicExpressionBooleanConstant.
     */
    
    class ParserEquals : public ExpressionParser {
    public:
        bool value; ParserEquals(bool value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionEquals(parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), value, source);
        }
    };
    
    class ParserSet : public ExpressionParser {
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'set' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            list<LogicExpression*> elems;
            for (unsigned i = 2; i < node->getNumOperands(); i++) {
                LogicExpression* elemExpr = ExpressionParser::parseMetadata(node->getOperand(i), source);
                elems.push_back(elemExpr);
            }
            return new LogicExpressionCreateSet(baseType, elems, source);
        }
    };
    
    class ParserTypeof : public ExpressionParser {
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            LogicExpression* expr = ExpressionParser::parseMetadata(exprNode, source);
            return new LogicExpressionConstantType(new LogicTypeType(expr->returnType(), source), source);
        }
    };
    
    class ParserResult : public ExpressionParser {
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 0);
            requireMaxArgs(node, exprName, source, 0);
            
            if (source->inst != NULL) {
                throw syntax_exception("'result' meaningless outside of function contract", NULL, source);
            }
            
            if (source->func->rawIR()->getReturnType()->isVoidTy()) {
                throw syntax_exception("Use of 'result' in function returning void", NULL, source);
            }
            return new LogicExpressionResult(new LogicTypeLLVM(source->func->rawIR()->getReturnType(), source), source);
        }
    };
    
    class ParserTrueFalse : public ExpressionParser {
    public:
        bool value; ParserTrueFalse(bool value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 0);
            requireMaxArgs(node, exprName, source, 0);
            return new LogicExpressionBooleanConstant(value, source);
        }
    };
    
    class ParserBool : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            LogicExpression* expr = ExpressionParser::parseMetadata(exprNode, source);
            return new LogicExpressionBoolean(expr, source);
        }
    };
    
    class ParserArg : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'arg' must be a string node", NULL, source);
            }
            string name = string(cast<MDString>(exprNode)->getString().data());
            return new LogicExpressionArgument(source->func->rawIR(), name, source);
        }
    };
    
    class ParserVar : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'arg' must be a string node", NULL, source);
            }
            string name = string(cast<MDString>(exprNode)->getString().data());
            if (source->inst == NULL) {
                throw syntax_exception("'var' meaningless inside of function contract", NULL, source);
            }
            return new LogicExpressionVariable(source->func->rawIR(), name, source);
        }
    };
    
    class ParserBinBool : public ExpressionParser {
    public:
        LogicExpressionBinaryBoolean::BinaryBooleanOp value; ParserBinBool(LogicExpressionBinaryBoolean::BinaryBooleanOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryBoolean(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserNot : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            return new LogicExpressionNot(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserIntConst : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'int' must be a string node", NULL, source);
            }
            string value = string(cast<MDString>(exprNode)->getString().data());
            return new LogicExpressionIntegerConstant(value, source);
        }
    };
    
    class ParserWAR : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'war' must be a string node", NULL, source);
            }
            string value = string(cast<MDString>(exprNode)->getString().data());
            return parseWarString(value, source);
        }
    };
    
    class ParserBinMath : public ExpressionParser {
    public:
        LogicExpressionBinaryMath::BinaryMathOp value; ParserBinMath(LogicExpressionBinaryMath::BinaryMathOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryMath(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserLocal : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'local' must be a string node", NULL, source);
            }
            string value = string(cast<MDString>(exprNode)->getString().data());
            return new LogicExpressionLocal(value, source);
        }
    };
    
    class ParserQuant : public ExpressionParser {
    public:
        bool forall; ParserQuant(bool forall) : forall{forall} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            Metadata* declListNodeRaw = node->getOperand(1).get();
            if (!isa<MDNode>(declListNodeRaw)) {
                throw syntax_exception(("Argument 1 of '" + string(exprName) + "' must be a list node"), NULL, source);
            }
            MDNode* declListNode = cast<MDNode>(declListNodeRaw);
            
            list<LogicLocal*>* locals = new list<LogicLocal*>();
            for (unsigned i = 0; i < declListNode->getNumOperands(); i++) {
                Metadata* declNodeRaw = declListNode->getOperand(i).get();
                if (!isa<MDNode>(declNodeRaw)) {
                    throw syntax_exception(("All elements of argument 1 of '" + string(exprName) + "' must be a list node"), NULL, source);
                }
                MDNode* declNode = cast<MDNode>(declNodeRaw);
                if (declNode->getNumOperands() != 2) {
                    throw syntax_exception(("Expected 2 arguments to elemtents of argument 1 of expression '" + string(exprName) + "'; got " + to_string(declNode->getNumOperands())), NULL, source);
                }
                Metadata* nameNodeRaw = declNode->getOperand(0).get();
                Metadata* typeNodeRaw = declNode->getOperand(1).get();
                
                if (!isa<MDString>(nameNodeRaw)) {
                    throw syntax_exception(("Element 0 of elements of argument 1 of '" + string(exprName) + "' must be a string node"), NULL, source);
                }
                string name = string(cast<MDString>(nameNodeRaw)->getString().data());
                
                LogicExpression* typeExpr = ExpressionParser::parseMetadata(typeNodeRaw, source);
                if (!isa<LogicTypeType>(typeExpr->returnType())) {
                    throw type_exception(("Element 1 of elements of argument 1 of '" + string(exprName) + "' must be of type 'type<T>', got type '" + typeExpr->returnType()->toString() + "'"), NULL, source);
                }
                LogicType* type = cast<LogicTypeType>(typeExpr->returnType())->getType();
                
                LogicLocal* local = new LogicLocal();
                local->name = name;
                local->type = type;
                locals->push_back(local);
            }
            
            NodeSource* newSource = new NodeSource(source);
            for (list<LogicLocal*>::iterator ii = locals->begin(); ii != locals->end(); ii++) {
                newSource->logicLocals[(*ii)->name].push_back(*ii);
            }
            LogicExpression* expr = ExpressionParser::parseMetadata(node->getOperand(2).get(), newSource);
            
            return new LogicExpressionQuantifier(forall, locals, expr, source);
        }
    };
    
    class ParserNeg : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            return new LogicExpressionNegate(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserIfThenElse : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 3);
            requireMaxArgs(node, exprName, source, 3);
            return new LogicExpressionConditional(parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), parseMetadata(node->getOperand(3).get(), source), source);
        }
    };
    
    class ParserLet : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            Metadata* declListNodeRaw = node->getOperand(1).get();
            if (!isa<MDNode>(declListNodeRaw)) {
                throw syntax_exception(("Argument 1 of '" + string(exprName) + "' must be a list node"), NULL, source);
            }
            MDNode* declListNode = cast<MDNode>(declListNodeRaw);
            
            list<pair<LogicLocal*, LogicExpression*>*>* locals = new list<pair<LogicLocal*, LogicExpression*>*>();
            for (unsigned i = 0; i < declListNode->getNumOperands(); i++) {
                Metadata* declNodeRaw = declListNode->getOperand(i).get();
                if (!isa<MDNode>(declNodeRaw)) {
                    throw syntax_exception(("All elements of argument 1 of '" + string(exprName) + "' must be a list node"), NULL, source);
                }
                MDNode* declNode = cast<MDNode>(declNodeRaw);
                if (declNode->getNumOperands() != 3) {
                    throw syntax_exception(("Expected 3 arguments to elemtents of argument 1 of expression '" + string(exprName) + "'; got " + to_string(declNode->getNumOperands())), NULL, source);
                }
                Metadata* nameNodeRaw = declNode->getOperand(0).get();
                Metadata* typeNodeRaw = declNode->getOperand(1).get();
                Metadata* valueNodeRaw = declNode->getOperand(2).get();
                
                if (!isa<MDString>(nameNodeRaw)) {
                    throw syntax_exception(("Element 0 of elements of argument 1 of '" + string(exprName) + "' must be a string node"), NULL, source);
                }
                string name = string(cast<MDString>(nameNodeRaw)->getString().data());
                
                LogicExpression* typeExpr = ExpressionParser::parseMetadata(typeNodeRaw, source);
                if (!isa<LogicTypeType>(typeExpr->returnType())) {
                    throw type_exception(("Element 1 of elements of argument 1 of '" + string(exprName) + "' must be of type 'type<T>', got type '" + typeExpr->returnType()->toString() + "'"), NULL, source);
                }
                LogicType* type = cast<LogicTypeType>(typeExpr->returnType())->getType();
                
                LogicExpression* valueExpr = ExpressionParser::parseMetadata(valueNodeRaw, source);
                
                LogicLocal* local = new LogicLocal();
                local->name = name;
                local->type = type;
                locals->push_back(new pair<LogicLocal*, LogicExpression*>(local, valueExpr));
            }
            
            NodeSource* newSource = new NodeSource(source);
            for (list<pair<LogicLocal*, LogicExpression*>*>::iterator ii = locals->begin(); ii != locals->end(); ii++) {
                newSource->logicLocals[(*ii)->first->name].push_back((*ii)->first);
            }
            LogicExpression* expr = ExpressionParser::parseMetadata(node->getOperand(2).get(), newSource);
            
            return new LogicExpressionLet(locals, expr, source);
        }
    };
    
    class ParserBinBits : public ExpressionParser {
    public:
        LogicExpressionBinaryBits::BinaryBitsOp value; ParserBinBits(LogicExpressionBinaryBits::BinaryBitsOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryBits(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserBinComp : public ExpressionParser {
    public:
        LogicExpressionBinaryCompare::BinaryCompareOp value; ParserBinComp(LogicExpressionBinaryCompare::BinaryCompareOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryCompare(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserBinCompLLVM : public ExpressionParser {
    public:
        LogicExpressionBinaryCompareLLVM::BinaryCompareLLVMOp value; ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::BinaryCompareLLVMOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryCompareLLVM(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserBitNot : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            return new LogicExpressionBitNot(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserBinShift : public ExpressionParser {
    public:
        LogicExpressionBinaryShift::BinaryShiftOp value; ParserBinShift(LogicExpressionBinaryShift::BinaryShiftOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryShift(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserTrunc : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'trunc' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            LogicExpression* expr = parseMetadata(node->getOperand(2).get(), source);
            if (isa<LogicTypeInt>(expr->returnType())) {
                return new LogicExpressionLogicIntToLLVMInt(expr, baseType, source);
            } else {
                return new LogicExpressionLLVMIntToLLVMInt(LogicExpressionLLVMIntToLLVMInt::OP_TRUNC, expr, baseType, source);
            }
        }
    };
    
    class ParserZext : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'zext' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            if (isa<LogicTypeInt>(baseType)) {
                return new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_ZEXT, parseMetadata(node->getOperand(2).get(), source), source);
            } else {
                return new LogicExpressionLLVMIntToLLVMInt(LogicExpressionLLVMIntToLLVMInt::OP_ZEXT, parseMetadata(node->getOperand(2).get(), source), baseType, source);
            }
        }
    };
    
    class ParserSext : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'sext' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            if (isa<LogicTypeInt>(baseType)) {
                return new LogicExpressionLLVMIntToLogicInt(LogicExpressionLLVMIntToLogicInt::OP_SEXT, parseMetadata(node->getOperand(2).get(), source), source);
            } else {
                return new LogicExpressionLLVMIntToLLVMInt(LogicExpressionLLVMIntToLLVMInt::OP_SEXT, parseMetadata(node->getOperand(2).get(), source), baseType, source);
            }
        }
    };
    
    class ParserRealConst : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            Metadata* exprNode = node->getOperand(1).get();
            if (!isa<MDString>(exprNode)) {
                throw syntax_exception("Argument 1 of 'real' must be a string node", NULL, source);
            }
            string value = string(cast<MDString>(exprNode)->getString().data());
            return new LogicExpressionRealConstant(value, source);
        }
    };
    
    class ParserRealToFloat : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'real.to.float' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            return new LogicExpressionRealToFloat(parseMetadata(node->getOperand(2).get(), source), baseType, source);
        }
    };
    
    class ParserFloatToReal : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            
            return new LogicExpressionFloatToReal(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserRealToInt : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            
            return new LogicExpressionRealToInt(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserIntToReal : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            
            return new LogicExpressionIntToReal(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserBinCompFloat : public ExpressionParser {
    public:
        LogicExpressionBinaryCompareFloat::BinaryCompareFloatOp value; ParserBinCompFloat(LogicExpressionBinaryCompareFloat::BinaryCompareFloatOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionBinaryCompareFloat(value, parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserLoad : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            return new LogicExpressionLoad(parseMetadata(node->getOperand(1).get(), source), source);
        }
    };
    
    class ParserGetIndex : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            return new LogicExpressionGetIndex(parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), source);
        }
    };
    
    class ParserSetIndex : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 3);
            requireMaxArgs(node, exprName, source, 3);
            return new LogicExpressionSetIndex(parseMetadata(node->getOperand(1).get(), source), parseMetadata(node->getOperand(2).get(), source), parseMetadata(node->getOperand(3).get(), source), source);
        }
    };
    
    class ParserArrayConst : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'array' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseTypeRaw = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            if (!isa<LogicTypeLLVM>(baseTypeRaw)) {
                throw type_exception(("Argument 1 to 'array' must be an LLVM type, got type '" + baseTypeRaw->toString() + "'"), NULL, source);
            }
            LogicTypeLLVM* baseType = cast<LogicTypeLLVM>(baseTypeRaw);
            
            list<LogicExpression*>* elems = new list<LogicExpression*>();
            for (unsigned i = 2; i < node->getNumOperands(); i++) {
                elems->push_back(parseMetadata(node->getOperand(i).get(), source));
            }
            
            return new LogicExpressionLLVMArrayConstant(baseType->getType(), elems, source);
        }
    };
    
    class ParserPtrToInt : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'ptr.to.int' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            return new LogicExpressionPointerToInt(parseMetadata(node->getOperand(2).get(), source), baseType, source);
        }
    };
    
    class ParserIntToPtr : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'int.to.ptr' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            return new LogicExpressionIntToPointer(parseMetadata(node->getOperand(2).get(), source), baseType, source);
        }
    };
    
    class ParserPtrToPtr : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'ptr.to.ptr' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            return new LogicExpressionPointerToPointer(parseMetadata(node->getOperand(2).get(), source), baseType, source);
        }
    };
    
    class ParserGEP : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            
            list<LogicExpression*>* elems = new list<LogicExpression*>();
            for (unsigned i = 2; i < node->getNumOperands(); i++) {
                elems->push_back(parseMetadata(node->getOperand(i).get(), source));
            }
            
            return new LogicExpressionGetElementPointer(parseMetadata(node->getOperand(1).get(), source), elems, source);
        }
    };
    
    class ParserSpecLLVMConst : public ExpressionParser {
    public:
        LogicExpressionSpecialLLVMConstant::SpecialLLVMConstOp value; ParserSpecLLVMConst(LogicExpressionSpecialLLVMConstant::SpecialLLVMConstOp value) : value{value} {}
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            requireMaxArgs(node, exprName, source, 1);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to '"+string(exprName)+"' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseType = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            return new LogicExpressionSpecialLLVMConstant(value, baseType, source);
        }
    };
    
    class ParserBaddr : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 2);
            requireMaxArgs(node, exprName, source, 2);
            
            Metadata* funcNode = node->getOperand(1).get();
            if (!isa<MDString>(funcNode)) {
                throw syntax_exception("Argument 1 of 'blockaddress' must be a string node", NULL, source);
            }
            string funcName = string(cast<MDString>(funcNode)->getString().data());
            
            Metadata* blockNode = node->getOperand(2).get();
            if (!isa<MDString>(blockNode)) {
                throw syntax_exception("Argument 2 of 'blockaddress' must be a string node", NULL, source);
            }
            string blockName = string(cast<MDString>(blockNode)->getString().data());
            
            Function* func = source->func->getModule()->rawIR()->getFunction(StringRef(funcName));
            if (!func) {
                throw syntax_exception("Argument 1 of 'blockaddress' must be the name of a function", NULL, source);
            }
            
            BasicBlock* block = NULL;
            for (Function::iterator ii = func->begin(); ii != func->end(); ii++) {
                if (ii->getName().compare(StringRef(blockName)) == 0) {
                    block = &*ii;
                    break;
                }
            }
            
            if (!block) {
                throw syntax_exception("Argument 2 of 'blockaddress' must be the name of a label within the function specified", NULL, source);
            }
            
            return new LogicExpressionBlockAddress(func, block, source);
        }
    };
    
    class ParserStructConst : public ExpressionParser {
    public:
        LogicExpression* parse(const char* exprName, MDNode* node, NodeSource* source) {
            requireMinArgs(node, exprName, source, 1);
            
            LogicExpression* baseTypeExpr = ExpressionParser::parseMetadata(node->getOperand(1).get(), source);
            if (!isa<LogicTypeType>(baseTypeExpr->returnType())) {
                throw type_exception(("Argument 1 to 'struct' must be of type 'type', got type '" + baseTypeExpr->returnType()->toString() + "'"), NULL, source);
            }
            LogicType* baseTypeRaw = cast<LogicTypeType>(baseTypeExpr->returnType())->getType();
            
            list<LogicExpression*>* elems = new list<LogicExpression*>();
            for (unsigned i = 2; i < node->getNumOperands(); i++) {
                elems->push_back(parseMetadata(node->getOperand(i).get(), source));
            }
            
            return new LogicExpressionLLVMStructConstant(baseTypeRaw, elems, source);
        }
    };
    
    /* ==========================
     * PARSER REGISTRY DATA TABLE
     * ==========================
     * 
     * Once an expression parser is made, it will need entries in the data table.
     * The key is the name to trigger the parser.
     * The value is a pointer to the parser itself.
     */
    
    static map<string,ExpressionParser*> parsers({
        {"eq",new ParserEquals(false)},
        {"neq",new ParserEquals(true)},
        {"set",new ParserSet()},
        {"typeof",new ParserTypeof()},
        {"result",new ParserResult()},
        {"true",new ParserTrueFalse(true)},
        {"false",new ParserTrueFalse(false)},
        {"bool",new ParserBool()},
        {"arg",new ParserArg()},
        {"var",new ParserVar()},
        {"and",new ParserBinBool(LogicExpressionBinaryBoolean::OP_AND)},
        {"or",new ParserBinBool(LogicExpressionBinaryBoolean::OP_OR)},
        {"->",new ParserBinBool(LogicExpressionBinaryBoolean::OP_IMPLIES)},
        {"<->",new ParserBinBool(LogicExpressionBinaryBoolean::OP_BIDIR_IMPLIES)},
        {"not",new ParserNot()},
        {"int",new ParserIntConst()},
        {"war",new ParserWAR()},
        {"add",new ParserBinMath(LogicExpressionBinaryMath::OP_ADD)},
        {"sub",new ParserBinMath(LogicExpressionBinaryMath::OP_SUB)},
        {"mul",new ParserBinMath(LogicExpressionBinaryMath::OP_MUL)},
        {"div",new ParserBinMath(LogicExpressionBinaryMath::OP_DIV)},
        {"mod",new ParserBinMath(LogicExpressionBinaryMath::OP_MOD)},
        {"rem",new ParserBinMath(LogicExpressionBinaryMath::OP_REM)},
        {"local",new ParserLocal()},
        {"forall",new ParserQuant(true)},
        {"exists",new ParserQuant(false)},
        {"neg",new ParserNeg()},
        {"ifte",new ParserIfThenElse()},
        {"let",new ParserLet()},
        {"band",new ParserBinBits(LogicExpressionBinaryBits::OP_AND)},
        {"bor",new ParserBinBits(LogicExpressionBinaryBits::OP_OR)},
        {"bxor",new ParserBinBits(LogicExpressionBinaryBits::OP_XOR)},
        {"sdiv",new ParserBinBits(LogicExpressionBinaryBits::OP_SDIV)},
        {"smod",new ParserBinBits(LogicExpressionBinaryBits::OP_SMOD)},
        {"srem",new ParserBinBits(LogicExpressionBinaryBits::OP_SREM)},
        {"udiv",new ParserBinBits(LogicExpressionBinaryBits::OP_UDIV)},
        {"umod",new ParserBinBits(LogicExpressionBinaryBits::OP_UMOD)},
        {"urem",new ParserBinBits(LogicExpressionBinaryBits::OP_UREM)},
        {"ge",new ParserBinComp(LogicExpressionBinaryCompare::OP_GE)},
        {"gt",new ParserBinComp(LogicExpressionBinaryCompare::OP_GT)},
        {"le",new ParserBinComp(LogicExpressionBinaryCompare::OP_LE)},
        {"lt",new ParserBinComp(LogicExpressionBinaryCompare::OP_LT)},
        {"sge",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_SGE)},
        {"sgt",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_SGT)},
        {"sle",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_SLE)},
        {"slt",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_SLT)},
        {"uge",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_UGE)},
        {"ugt",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_UGT)},
        {"ule",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_ULE)},
        {"ult",new ParserBinCompLLVM(LogicExpressionBinaryCompareLLVM::OP_ULT)},
        {"bnot",new ParserBitNot()},
        {"ashr",new ParserBinShift(LogicExpressionBinaryShift::OP_ASHR)},
        {"shl",new ParserBinShift(LogicExpressionBinaryShift::OP_LSHL)},
        {"lshr",new ParserBinShift(LogicExpressionBinaryShift::OP_LSHR)},
        {"trunc",new ParserTrunc()},
        {"zext",new ParserZext()},
        {"sext",new ParserSext()},
        {"real",new ParserRealConst()},
        {"float.to.real",new ParserFloatToReal()},
        {"real.to.float",new ParserRealToFloat()},
        {"real.to.int",new ParserRealToInt()},
        {"int.to.real",new ParserIntToReal()},
        {"foeq",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_OEQ)},
        {"fogt",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_OGT)},
        {"foge",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_OGE)},
        {"folt",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_OLT)},
        {"fole",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_OLE)},
        {"fone",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_ONE)},
        {"ford",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_ORD)},
        {"fueq",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_UEQ)},
        {"fugt",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_UGT)},
        {"fuge",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_UGE)},
        {"fult",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_ULT)},
        {"fule",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_ULE)},
        {"fune",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_UNE)},
        {"funo",new ParserBinCompFloat(LogicExpressionBinaryCompareFloat::OP_UNO)},
        {"load",new ParserLoad()},
        {"get",new ParserGetIndex()},
        {"set",new ParserSetIndex()},
        {"array",new ParserArrayConst()},
        {"ptr.to.int",new ParserPtrToInt()},
        {"int.to.ptr",new ParserIntToPtr()},
        {"ptr.to.ptr",new ParserPtrToPtr()},
        {"getelementptr",new ParserGEP()},
        {"maxint",new ParserSpecLLVMConst(LogicExpressionSpecialLLVMConstant::OP_MAXINT)},
        {"minint",new ParserSpecLLVMConst(LogicExpressionSpecialLLVMConstant::OP_MININT)},
        {"maxuint",new ParserSpecLLVMConst(LogicExpressionSpecialLLVMConstant::OP_MAXUINT)},
        {"minuint",new ParserSpecLLVMConst(LogicExpressionSpecialLLVMConstant::OP_MINUINT)},
        {"blockaddress",new ParserBaddr()},
        {"struct",new ParserStructConst()},
    });
    map<string,ExpressionParser*>* ExpressionParser::getExpressionParsers() {
        return &parsers;
    }
}
