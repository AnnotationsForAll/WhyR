/*
 * test_war_prec.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: jrobbins
 */

#include "test_common.hpp"

#include <whyr/module.hpp>
#include <whyr/exception.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/war.hpp>

class WarPrecTests: public ::testing::Test {
protected:
    llvm::LLVMContext* ctx;
    std::unique_ptr<llvm::Module> llvmMod;
    llvm::Function* llvmFunc;
    whyr::AnnotatedModule* mod;
    whyr::AnnotatedFunction* func;
    whyr::NodeSource* source;
    
    virtual void SetUp() {
        using namespace std; using namespace llvm; using namespace whyr;
        
        ctx = new LLVMContext();
        llvmMod = unique_ptr<Module>(new Module(StringRef("<test case>"), *ctx));
        llvmFunc = cast<Function>(llvmMod->getOrInsertFunction(StringRef("test"), Type::getInt32Ty(*ctx), NULL));
        
        mod = new AnnotatedModule(llvmMod);
        func = new AnnotatedFunction(mod, llvmFunc);
        
        source = new NodeSource(func);
    }
    
    virtual void TearDown() {
        using namespace std; using namespace llvm; using namespace whyr;
        
        delete mod;
    }
};

#define WAR_PREC_TEST(test_name, test_string, code) TEST_F(WarPrecTests,test_name) {    \
    using namespace std; using namespace llvm; using namespace whyr;                    \
    ASSERT_NO_THROW({                                                                   \
        try {                                                                           \
            LogicExpression* expr = parseWarString(test_string, source);                \
            expr->checkTypes();                                                         \
            code                                                                        \
        } catch (whyr_exception ex) {                                                   \
            string errMsg = string("'") + ex.what() + "'";                              \
            FAIL_WITH_MESSAGE(errMsg);                                                  \
        }                                                                               \
    });                                                                                 \
}

WAR_PREC_TEST(TestAddMul, "1 + 2 * 3", {
        ASSERT_TRUE(isa<LogicExpressionBinaryMath>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryMath>(expr)->getOp(), LogicExpressionBinaryMath::OP_ADD);
})

WAR_PREC_TEST(TestSubDiv, "1 / 2 - 3", {
        ASSERT_TRUE(isa<LogicExpressionBinaryMath>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryMath>(expr)->getOp(), LogicExpressionBinaryMath::OP_SUB);
})

WAR_PREC_TEST(TestSubSdiv, "(i32)1 sdiv (i32)2 - (i32)3", {
        ASSERT_TRUE(isa<LogicExpressionBinaryMath>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryMath>(expr)->getOp(), LogicExpressionBinaryMath::OP_SUB);
})

WAR_PREC_TEST(TestCondNot, "!false?1:2", {
        ASSERT_TRUE(isa<LogicExpressionConditional>(expr));
})

WAR_PREC_TEST(TestAndNot, "!true && false", {
        ASSERT_TRUE(isa<LogicExpressionBinaryBoolean>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryBoolean>(expr)->getOp(), LogicExpressionBinaryBoolean::OP_AND);
})

WAR_PREC_TEST(TestAndOr, "true || true && false || true", {
        ASSERT_TRUE(isa<LogicExpressionBinaryBoolean>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryBoolean>(expr)->getOp(), LogicExpressionBinaryBoolean::OP_OR);
})

WAR_PREC_TEST(TestAndImp, "true && false ==> true || false", {
        ASSERT_TRUE(isa<LogicExpressionBinaryBoolean>(expr));
        ASSERT_EQ(cast<LogicExpressionBinaryBoolean>(expr)->getOp(), LogicExpressionBinaryBoolean::OP_IMPLIES);
})
