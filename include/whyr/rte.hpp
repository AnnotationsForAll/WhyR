/*
 * rte.hpp
 *
 *  Created on: Oct 13, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_RTE_HPP_
#define INCLUDE_WHYR_RTE_HPP_

/**
 * This header contains functions to annotate LLVM programs with runtime error assertions (RTEs for short).
 * Some conditions in the program, such as out-of-bounds memeory access or division by zero, may cause undefined behavior.
 * Adding RTEs to a program will prove that they will not evoke undefined behavior when called.
 */

#include "module.hpp"

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    /**
     * Annotates an instruction with RTE assertions.
     */
    void addRTE(AnnotatedFunction* func, Instruction* inst);
    
    /**
     * Annotates a function with RTE assertions.
     */
    void addRTE(AnnotatedFunction* func);
    
    /**
     * Annotates a module with RTE assertions.
     */
    void addRTE(AnnotatedModule* module);
}

#endif /* INCLUDE_WHYR_RTE_HPP_ */
