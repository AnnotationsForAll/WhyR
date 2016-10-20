/*
 * exec_why3.hpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jrobbins
 */

#ifndef INCLUDE_WHYR_EXEC_WHY3_HPP_
#define INCLUDE_WHYR_EXEC_WHY3_HPP_

/**
 * This header contains utilities to execute a Why3 process and parse the proof results.
 */

#include "whyr.hpp"

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    static const string PROVER_ALT_ERGO("alt-ergo");
    static const string PROVER_Z3("z3");
    static const string PROVER_CVC4("cvc4");
    
    /// This represents a single why3 goal.
    struct Why3Goal {
        /// The theory the goal belongs to.
        char* theory = NULL;
        /// The name of the goal.
        char* goal = NULL;
        /// Whether or not the goal verified, failed, or timed out.
        enum Why3GoalStatus {
            STATUS_VALID,
            STATUS_UNKNOWN,
            STATUS_FAIL,
            STATUS_TIMEOUT,
        } status;
        /// The time, in seconds, for the goal to finish.
        double time;
        /// The number of steps it took to prove the goal. If the prover isn't alt-ergo, or the goal didn't pass, this will be -1.
        int steps = -1;
    };
    
    /// This represents the whole output of a Why3 proving session.
    struct Why3Output {
        /// If true, an error occurred.
        bool error = false;
        /// The error message. NULL if error is false.
        char* message = NULL;
        /// The line the error occurred on. This value is undefined if error is false.
        int line = -1;
        /// The beginning and end columns the error occurred on. This value is undefined if error is false.
        int colBegin = -1; int colEnd = -1;
        /// The list of goals that were attempted.
        list<Why3Goal> goals;
        
        /**
         * str is the raw output of a why3 proof. Get this string via execWhy3.
         */
        Why3Output(const char* str);
        ~Why3Output();
    };
    
    /**
     * Takes a Why3-format string (NOT a filename!), and places the raw output of Why3 into out.
     * set checkOnly to true if you don't want to prove anything, only check the program is correct.
     * prover is the prover you want to use. If not specified, defaults to Alt-Ergo.
     */
    void execWhy3(string &in, ostream &out, bool checkOnly = false, const string &prover = PROVER_ALT_ERGO);
}

#endif /* INCLUDE_WHYR_EXEC_WHY3_HPP_ */
