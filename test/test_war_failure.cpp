/*
 * test_war.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jrobbins
 */

#include "test_common.hpp"

#include <whyr/module.hpp>
#include <whyr/exception.hpp>
#include <whyr/expressions.hpp>
#include <whyr/types.hpp>
#include <whyr/war.hpp>

static llvm::LLVMContext* ctx;
static std::unique_ptr<llvm::Module> llvmMod;
static llvm::Function* llvmFunc;
static whyr::AnnotatedModule* mod;
static whyr::AnnotatedFunction* func;

class WarFailureTests : public ::testing::TestWithParam<std::string> {};

TEST_P(WarFailureTests,) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    ASSERT_THROW({
            NodeSource* source = new NodeSource(func);
            
            LogicExpression* expr = parseWarString(GetParam(), source);
            expr->checkTypes();
    }, whyr_exception);
}

/**
 * This is guaranteed to run before the INSTANTIATE_TEST_CASE_P is initialized.
 * This allows us to use the LLVMContext in initializers that need it.
 */
static void setup() {
    using namespace std; using namespace llvm; using namespace whyr;
    
    ctx = new LLVMContext();
    llvmMod = unique_ptr<Module>(new Module(StringRef("<test case>"), *ctx));
    llvmFunc = cast<Function>(llvmMod->getOrInsertFunction(StringRef("test"), Type::getInt32Ty(*ctx), NULL));
    
    mod = new AnnotatedModule(llvmMod);
    func = new AnnotatedFunction(mod, llvmFunc);
}

INSTANTIATE_TEST_CASE_P(,WarFailureTests,(setup(), ::testing::ValuesIn((std::string[]){
    "1 + (i32)1",
    "+1",
    "1+ +2",
    "1 + 2 * (i32)4 sdiv 3",
    "? 3 : 4",
    "forall int $x; $x",
    "result == 2",
    "32 shl 8",
    "(i32)32 ashr (i32)8",
    "(int) (i32) 42",
    "(i64) (i32) 42",
    "(i32) (i64 zext) 42",
    "(i32 sext) (i64) 42",
    "(bool zext) (i1) false",
    "0.",
    "1 + 1.0",
    "(i32) 1.1",
    "(double) 12",
    "(float)1.2 <= (float)4.5",
    "1 funo 2",
    "null",
    "(real*) null",
    "3[4]",
    "{1,2,3}",
    "{(i32)1,(double)2.2}",
    "{1,2.0,3}",
    "{}",
    "(float){}",
    "(int){}",
    "(int[0]){}",
    "(i32[1]){}",
    "(int[]){}",
    "{(i32)0}[(i32)0 = (i32)0]",
    "{(i32)0}[0 = 0]",
    "(int)(i32*)null",
    "(i32*)0",
    "((i32*)null)[1.2]",
    "&42",
    "&((i32*)null)[1][2][3]",
    "min",
    "(i32) max",
    "(int zext) min",
    "blockaddress(@test, %label)",
    "blockaddress(@test, @label)",
    "blockaddress(42, %label)",
    "label: ",
    "label: label: true",
    "struct {(i32)4}",
    "(struct {i32}) struct {4}",
    "(struct {i32}) struct {(i32)1, (i32)2}",
})));
