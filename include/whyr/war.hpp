/*
 * war.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_WAR_HPP_
#define INCLUDE_WHYR_WAR_HPP_

/**
 * The WhyR Annotation Language (WAR) is a high-level language for annotations that can be translated down into WhyR LogicExpressions.
 */

#include "logic.hpp"

#include <string>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    LogicExpression* parseWarString(string war, NodeSource* source);
}

#endif /* INCLUDE_WHYR_WAR_HPP_ */
