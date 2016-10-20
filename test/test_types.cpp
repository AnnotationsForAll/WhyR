/*
 * test_types.cpp
 *
 *  Created on: Sep 20, 2016
 *      Author: jrobbins
 */

#include "gtest/gtest.h"

#include <whyr/expressions.hpp>
#include <whyr/types.hpp>

TEST(TypesTests, TestTrueIsBool) {
    using namespace whyr;
    LogicExpression* expr = new LogicExpressionBooleanConstant(true);
    ASSERT_TRUE(isa<LogicTypeBool>(expr->returnType()));
    delete expr;
}
