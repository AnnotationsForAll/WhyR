/*
 * exec_why3.cpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jrobbins
 */

#include <whyr/exec_why3.hpp>
#include <whyr/exception.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <list>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    void execWhy3(string &in, ostream &out, bool checkOnly, const string &prover) {
        pid_t pid;
        int rv;
        int inpipe[2];
        int outpipe[2];
        
        if (pipe(inpipe) || pipe(outpipe)) {
            throw whyr_exception("when executing why3: pipe() failed");
        }
        
        pid = fork();
        if (pid == -1) {
            throw whyr_exception("when executing why3: fork() failed");
        }
        
        if (pid) {
            // parent
            close(inpipe[0]);
            close(outpipe[1]);
            // prints here give input to child via inpipe[1], the writey end of the in-pipe
            write(inpipe[1], in.c_str(), in.size());
            // done writing input, wait for end of process
            close(inpipe[1]);
            waitpid(pid, &rv, 0);
            // reads here are output of child via outpipe[0], the ready end of the out-pipe
            char buf[128];
            ssize_t n;
            do {
                n = read(outpipe[0], buf, 127);
                if (n == -1) {
                    throw whyr_exception("when executing why3: read() failed");
                }
                buf[n] = '\0';
                out << buf;
            } while (n > 0);
            
            close(outpipe[0]);
        } else {
            // child
            dup2(inpipe[0], 0); // stdin = ready end of in-pipe
            dup2(outpipe[1], 1); // stdout + stderr = writey end of out-pipe
            dup2(outpipe[1], 2);
            
            close(inpipe[1]);
            close(outpipe[0]);
            
            if (checkOnly) {
                if (execlp("why3", "why3", "prove", "-F", "why", "--type-only", "-", NULL) == -1) {
                    throw whyr_exception("when executing why3: execlp() failed");
                }
            } else {
                if (execlp("why3", "why3", "prove", "-P", prover.c_str(), "-F", "why", "-a", "inline_all", "-", NULL) == -1) {
                    throw whyr_exception("when executing why3: execlp() failed");
                }
            }
        }
    }
    
    Why3Output::Why3Output(const char* str) {
        while (strncmp(str, "File \"", 6) == 0) {
            // grab line info in case it is an error
            size_t n;
            char* lineStr;
            
            n = strcspn(str, ",");
            str += n + 7;
            
            n = strcspn(str, ",");
            lineStr = strndup(str, n);
            line = strtoul(lineStr, NULL, 0);
            free(lineStr);
            str += n + 14;
            
            n = strcspn(str, "-");
            lineStr = strndup(str, n);
            colBegin = strtoul(lineStr, NULL, 0);
            free(lineStr);
            str += n + 1;
            
            n = strcspn(str, ":");
            lineStr = strndup(str, n);
            colEnd = strtoul(lineStr, NULL, 0);
            free(lineStr);
            
            // either an error or a warning.
            while (*str != '\n' && *str != '\0') {
                str++;
            }
            if (*str == '\0') {
                error = true;
                return;
            }
            str++;
            if (strncmp(str, "warning:", 8) == 0) {
                // warning; ignore it
                while (*str != '\n' && *str != '\0') {
                    str++;
                }
                if (*str == '\0') {
                    error = true;
                    return;
                }
                str++;
            } else {
                // error
                error = true;
                
                n = strcspn(str, "\n");
                message = strndup(str, n);
                return;
            }
        }
        
        // no error
        error = false;
        
        while (*str != '\0') {
            Why3Goal goal;
            size_t n;
            
            n = strcspn(str, " ");
            str += n + 1;
            
            n = strcspn(str, " ");
            goal.theory = strndup(str, n);
            str += n + 1;
            
            n = strcspn(str, " ");
            goal.goal = strndup(str, n);
            str += n + 3;
            
            n = strcspn(str, " ");
            if (strncmp(str, "Valid", n) == 0) {
                goal.status = Why3Goal::STATUS_VALID;
            } else if (strncmp(str, "Unknown", n) == 0) {
                goal.status = Why3Goal::STATUS_UNKNOWN;
            } else if (strncmp(str, "Timeout", n) == 0) {
                goal.status = Why3Goal::STATUS_TIMEOUT;
            } else if (strncmp(str, "Failure", n) == 0) {
                goal.status = Why3Goal::STATUS_FAIL;
            } else if (strncmp(str, "HighFailure", n) == 0) {
                // if we had a high failure, the rest of the output is essentially unparsable. Don't even try.
                goal.status = Why3Goal::STATUS_FAIL;
                goals.push_back(goal);
                return;
            }
            str += n + 2;
            
            n = strcspn(str, ",)");
            char* timeString = strndup(str, n);
            goal.time = strtod(timeString, NULL);
            free(timeString);
            str += n;
            
            if (*str == ',') {
                // we have steps info
                str++;
                n = strcspn(str, ")");
                char* stepsString = strndup(str, n);
                goal.steps = strtoul(stepsString, NULL, 0);
                free(stepsString);
            }
            
            goals.push_back(goal);
            
            // scan to end of line
            while (*str != '\n' && *str != '\0') {
                str++;
            }
            if (*str != '\0') {
                str++;
            }
        }
    }
    
    Why3Output::~Why3Output() {
        if (message) free(message);
        for (list<Why3Goal>::iterator ii = goals.begin(); ii != goals.end(); ii++) {
            if (ii->theory) free(ii->theory);
            if (ii->goal) free(ii->goal);
        }
    }
}

