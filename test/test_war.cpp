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

struct WarTestData {
    std::string input;
    whyr::LogicType* type;
};

std::ostream& operator<<(std::ostream &out, const WarTestData &data) {
    return out << "'" << data.input << "'";
}

static llvm::LLVMContext* ctx;
static std::unique_ptr<llvm::Module> llvmMod;
static llvm::Function* llvmFunc;
static whyr::AnnotatedModule* mod;
static whyr::AnnotatedFunction* func;

class WarTests : public ::testing::TestWithParam<WarTestData> {};

static bool predicate_type_equals(whyr::LogicType* a, whyr::LogicType* b) {
    return a->equals(b);
}

TEST_P(WarTests,) {
    using namespace std; using namespace llvm; using namespace whyr;
    
    ASSERT_NO_THROW({
        try {
            NodeSource* source = new NodeSource(func);
            
            LogicExpression* expr = parseWarString(GetParam().input, source);
            ASSERT_PRED2(predicate_type_equals, expr->returnType(), GetParam().type);
            expr->checkTypes();
        } catch (whyr_exception ex) {
            string errMsg = string("'") + ex.what() + "'";
            FAIL_WITH_MESSAGE(errMsg);
        }
    });
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

INSTANTIATE_TEST_CASE_P(,WarTests,(setup(), ::testing::ValuesIn((WarTestData[]){
    {"1", new whyr::LogicTypeInt()},
    {"   1\n \t ", new whyr::LogicTypeInt()},
    {"true", new whyr::LogicTypeBool()},
    {"false", new whyr::LogicTypeBool()},
    {"(i32) 42", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(1)", new whyr::LogicTypeInt()},
    {"(((((1)))))", new whyr::LogicTypeInt()},
    {"((i32) (42))", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"true && false", new whyr::LogicTypeBool()},
    {"true ==> false || true <==> (true && false) ==> false", new whyr::LogicTypeBool()},
    {"!true", new whyr::LogicTypeBool()},
    {"!!!true && false", new whyr::LogicTypeBool()},
    {"!(!!true && false) ==> !false", new whyr::LogicTypeBool()},
    {"1 == 2", new whyr::LogicTypeBool()},
    {"3 != 4", new whyr::LogicTypeBool()},
    {"1 != 2 == true", new whyr::LogicTypeBool()},
    {"1 + 1 == 2", new whyr::LogicTypeBool()},
    {"1 + 2 * 3", new whyr::LogicTypeInt()},
    {"(i64)1 + (i64)1", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,64))},
    {"forall bool $i; $i", new whyr::LogicTypeBool()},
    {"exists int $a; exists int $b; $a == $b", new whyr::LogicTypeBool()},
    {"forall i4 $\"x\"; exists i4 $\"y\"; !($x != $y)", new whyr::LogicTypeBool()},
    {"1 + 1 == 2 + 2", new whyr::LogicTypeBool()},
    {"-1", new whyr::LogicTypeInt()},
    {"-(1+1)", new whyr::LogicTypeInt()},
    {"-1+1", new whyr::LogicTypeInt()},
    {"-(i20)1", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,20))},
    {"true ? false : true", new whyr::LogicTypeBool()},
    {"true ? 1 : 0", new whyr::LogicTypeInt()},
    {"true ? 1 : false ? 2 : 3", new whyr::LogicTypeInt()},
    {"true ? false ? 1 : 2 : 3", new whyr::LogicTypeInt()},
    {"let int $x = 2; $x", new whyr::LogicTypeInt()},
    {"let int $x = 2, int $y = 3; $x == $y", new whyr::LogicTypeBool()},
    {"let int $x = 2; let int $y = 3; $x == $y", new whyr::LogicTypeBool()},
    {"result", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"result + (i32)1", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"12 mod 2 == 0", new whyr::LogicTypeBool()},
    {"(i32)12 sdiv (i32)2 == (i32)12 udiv (i32)2", new whyr::LogicTypeBool()},
    {"(i32)1 & (i32)2 | (i32)3", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"1 < 2", new whyr::LogicTypeBool()},
    {"1 > 2 ==> 3 <= 4", new whyr::LogicTypeBool()},
    {"(i32)1 ugt (i32)2 ==> (i32)3 ule (i32)4", new whyr::LogicTypeBool()},
    {"(i32)1 sgt (i32)2 ==> (i32)3 sle (i32)4", new whyr::LogicTypeBool()},
    {"~~(i32)1 | ~(i32)2 & ~~~(i32)3", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32)42 shl 4", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32)42 lshr 4", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32)42 ashr 4 shl 4", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i1)false", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,1))},
    {"(i32)true", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(bool) (i1) true", new whyr::LogicTypeBool()},
    {"(i32)(1+1)", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32) (i64) 44", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i1) (i2) (i3) (i4) 5", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,1))},
    {"(i32 zext) (i1) true", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32 sext) (i16) 42", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(int zext) (i64) 120", new whyr::LogicTypeInt()},
    {"1 + (int sext) (i128) (1+1)", new whyr::LogicTypeInt()},
    {"0.0", new whyr::LogicTypeReal()},
    {"3.14159", new whyr::LogicTypeReal()},
    {".00001", new whyr::LogicTypeReal()},
    {"2.0 == 2.0", new whyr::LogicTypeBool()},
    {"let real $r = .0; $r", new whyr::LogicTypeReal()},
    {"2.0 + 2.0", new whyr::LogicTypeReal()},
    {"(double) 2.0", new whyr::LogicTypeLLVM(llvm::Type::getDoubleTy(*ctx))},
    {"(int) (real) (double) 1.5", new whyr::LogicTypeInt()},
    {"(float) (real) (int) (real) (double) (1.1 * 0.5)", new whyr::LogicTypeLLVM(llvm::Type::getFloatTy(*ctx))},
    {"(float) (double) 66.6", new whyr::LogicTypeLLVM(llvm::Type::getFloatTy(*ctx))},
    {"-1.234", new whyr::LogicTypeReal()},
    {"- (float) 1.234", new whyr::LogicTypeLLVM(llvm::Type::getFloatTy(*ctx))},
    {"1.2 <= 4.5", new whyr::LogicTypeBool()},
    {"(float)1.2 foeq (float)4.5", new whyr::LogicTypeBool()},
    {"(double)1.2 funo (double)4.5", new whyr::LogicTypeBool()},
    {"(float)1.2 fogt (float)(4.5 + .01)", new whyr::LogicTypeBool()},
    {"(i32*)null", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,32), 0))},
    {"(float*)null", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getFloatTy(*ctx), 0))},
    {"{ (i32)1 }", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getIntNTy(*ctx,32), 1))},
    {"{ (i32)1, (i32)2, (i32)6 }", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getIntNTy(*ctx,32), 3))},
    {"{ (float)1.2, (float)0.1 }", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getFloatTy(*ctx), 2))},
    {"(i32[0]) {}", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getIntNTy(*ctx,32), 0))},
    {"(float[0]) {}", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getFloatTy(*ctx), 0))},
    {"(i32[]) {}", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getIntNTy(*ctx,32), 0))},
    {"(float[]) {}", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getFloatTy(*ctx), 0))},
    {"{(i32)1}[0 = (i32)5]", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getIntNTy(*ctx,32), 1))},
    {"{(float)1.0,(float)2.0}[(0 + 1) = (float)3.0]", new whyr::LogicTypeLLVM(llvm::ArrayType::get(llvm::Type::getFloatTy(*ctx), 2))},
    {"(i64)(i8*)null", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,64))},
    {"(i32)(i16[]*)null", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i8*)(i64)0", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,8), 0))},
    {"(i8*)(i32)0", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,8), 0))},
    {"(i8*)(i32[4]*)null", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,8), 0))},
    {"&(i32*)null", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,32), 0), 0)},
    {"&((i32*)null)[0]", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,32), 0))},
    {"&((i32[5]*)null)[0][2]", new whyr::LogicTypeLLVM(llvm::PointerType::get(llvm::Type::getIntNTy(*ctx,32), 0))},
    {"(i32 zext) max", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32 sext) max", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32 zext) min", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"(i32 sext) min", new whyr::LogicTypeLLVM(llvm::Type::getIntNTy(*ctx,32))},
    {"label: true", new whyr::LogicTypeBool()},
    {"label: true ? false : true", new whyr::LogicTypeBool()},
})));
