/*
 * test_common.hpp
 *
 *  Created on: Sep 27, 2016
 *      Author: jrobbins
 */

#ifndef TEST_TEST_COMMON_HPP_
#define TEST_TEST_COMMON_HPP_

#include "gtest/gtest.h"

#include <string>

bool fail_with_message(std::string m);
#define FAIL_WITH_MESSAGE(m) ASSERT_PRED1(fail_with_message, m);
#define WARN_WITH_MESSAGE(m) EXPECT_PRED1(fail_with_message, m);

#endif /* TEST_TEST_COMMON_HPP_ */
