/*
 * main.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

/*
This is where main() lies, as well as general functions for using this application on the command line.
*/

#include <whyr/exception.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/module.hpp>
#include <whyr/rte.hpp>
#include <whyr/exec_why3.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include "optionparser.h"

static option::ArgStatus requireArgument(const option::Option& option, bool msg) {
    if (!option.arg) {
        std::cerr << "error: option " << option.name << " requires an argument" << std::endl;
        exit(1);
    }
    return option::ArgStatus::ARG_OK;
}

enum Options {
    UNKNOWN,
    HELP,
    VERSION,
    OUTPUT,
    WHY3_INT_MODE,
    WHY3_FLOAT_MODE,
    WERROR,
    NO_WARN,
    WHY3_MEM_MODEL,
    ENABLE_RTE,
    DISABLE_GOALS,
    COMBINE_GOALS,
    INPUT_FORMAT,
    VACUOUS_CHECKS,
    PROVE,
    PROVER,
};
static const option::Descriptor usage[] = {
    { UNKNOWN, 0, "", "", option::Arg::None,                        "USAGE: whyr [<option>...] <file>" },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "OPTIONS:" },
    { HELP, 0, "h", "help", option::Arg::None,                      "    --help (-h)           - Prints this help information." },
    { VERSION, 0, "v", "version", option::Arg::None,                "    --version (-v)        - Prints version information and exits." },
    { OUTPUT, 0, "o", "output-file", requireArgument,               "    --output-file (-o)    - Send output to the given file." },
    { WHY3_INT_MODE, 0, "", "why3-ints", requireArgument,           "    --why3-ints           - Change how LLVM integers are modeled in Why3." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            Valid values: 'int', 'bv'" },
    { WHY3_FLOAT_MODE, 0, "", "why3-floats", requireArgument,       "    --why3-floats         - Change how LLVM floats are modeled in Why3." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            Valid values: 'real', 'fp'" },
    { WERROR, 0, "W", "werror", option::Arg::None,                  "    --werror (-W)         - Make all warnings treated as errors." },
    { NO_WARN, 0, "w", "no-warn", option::Arg::None,                "    --no-warn (-w)        - Disable warning messages." },
    { WHY3_MEM_MODEL, 0, "m", "why3-model", requireArgument,        "    --why3-model (-m)     - Sets the memory model, used for proving programs with state." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            Valid values: 'default', 'dummy'" },
    { ENABLE_RTE, 0, "r", "rte", option::Arg::None,                 "    --rte (-r)            - Enables RTE assertion generation." },
    { DISABLE_GOALS, 0, "g", "no-goals", option::Arg::None,         "    --no-goals (-g)       - Disables goal generation. Only generates theories." },
    { COMBINE_GOALS, 0, "G", "combine-goals", option::Arg::None,    "    --combine-goals (-G)  - For each function, combines all goals into one." },
    { INPUT_FORMAT, 0, "f", "format", requireArgument,              "    --format (-f)         - Change what input format WhyR reads input files as." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            Valid values: 'auto', 'bc', 'll'" },
    { VACUOUS_CHECKS, 0, "V", "vacuous-checks", option::Arg::None,  "    --vacuous-checks (-V) - If specified, adds vacuous assertions to all goals." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            A vacuous goal is intended to fail or time out." },
    { UNKNOWN, 0, "", "", option::Arg::None,                        "                            if a vacuous goal passes, there is a contradiction in logic." },
    { PROVE, 0, "p", "prove", option::Arg::None,                    "    --prove (-p)          - If specified, runs output through Why3 and displays results." },
    { PROVER, 0, "P", "prover", requireArgument,                    "    --prover (-P)         - Specify the prover to run with '-p'. Default is 'alt-ergo'." },
    { 0, 0, 0, 0, 0, 0 }
};

int main(int argc, char** argv) {
    argc-=(argc>0); argv+=(argc>0);
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max];
    option::Option buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);
    
    if (parse.error()) {
        return 1;
    }
    
    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }
    
    if (options[VERSION]) {
        std::cout << "WhyR (version " << WHYR_VERSION << ")" << std::endl;
        std::cout << "https://github.com/AnnotationsForAll/WhyR" << std::endl;
        return 0;
    }
    
    if (options[UNKNOWN]) {
        for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next()) {
            std::cout << "error: unknown option " << opt->name << std::endl;
        }
        return 1;
    }
    
    const char* input_file;
    if (parse.nonOptionsCount() > 0) {
        input_file = parse.nonOption(0);
    } else {
        option::printUsage(std::cout, usage);
        return 0;
    }
    
    whyr::WhyRSettings settings;
    
    if (options[WHY3_INT_MODE]) {
        std::string optstr(options[WHY3_INT_MODE].arg);
        if (optstr.compare("int") == 0) {
            settings.why3IntMode = whyr::WhyRSettings::WHY3_INT_MODE_INT;
        } else if (optstr.compare("bv") == 0) {
            settings.why3IntMode = whyr::WhyRSettings::WHY3_INT_MODE_BV;
        } else {
            std::cerr << "error: invalid option to " << options[WHY3_INT_MODE].name << ": Unknown why3 int mode '" << optstr << "'" << std::endl;
            return 1;
        }
    }
    
    if (options[WHY3_FLOAT_MODE]) {
        std::string optstr(options[WHY3_FLOAT_MODE].arg);
        if (optstr.compare("real") == 0) {
            settings.why3FloatMode = whyr::WhyRSettings::WHY3_FLOAT_MODE_REAL;
        } else if (optstr.compare("fp") == 0) {
            settings.why3FloatMode = whyr::WhyRSettings::WHY3_FLOAT_MODE_FP;
        } else {
            std::cerr << "error: invalid option to " << options[WHY3_FLOAT_MODE].name << ": Unknown why3 float mode '" << optstr << "'" << std::endl;
            return 1;
        }
    }
    
    if (options[WERROR]) settings.werror = true;
    if (options[NO_WARN]) settings.noWarn = true;
    if (options[ENABLE_RTE]) settings.rte = true;
    if (options[DISABLE_GOALS]) settings.noGoals = true;
    if (options[COMBINE_GOALS]) settings.combineGoals = true;
    if (options[VACUOUS_CHECKS]) settings.vacuousChecks = true;
    
    if (options[WHY3_MEM_MODEL]) {
        std::string optstr(options[WHY3_MEM_MODEL].arg);
        if (optstr.compare("default") == 0) {
            settings.why3MemModel = whyr::WhyRSettings::WHY3_MEM_MODEL_DEFAULT;
        } else if (optstr.compare("dummy") == 0) {
            settings.why3MemModel = whyr::WhyRSettings::WHY3_MEM_MODEL_DUMMY;
        } else {
            std::cerr << "error: invalid option to " << options[WHY3_MEM_MODEL].name << ": Unknown why3 memory model '" << optstr << "'" << std::endl;
            return 1;
        }
    }
    
    whyr::AnnotatedModule* (*modFunc)(std::istream&, const char*, whyr::WhyRSettings*) = whyr::AnnotatedModule::moduleFromIR;
    if (options[INPUT_FORMAT]) {
        std::string optstr(options[INPUT_FORMAT].arg);
        if (optstr.compare("auto") == 0) {
            modFunc = whyr::AnnotatedModule::moduleFromIR;
        } else if (optstr.compare("bc") == 0) {
            modFunc = whyr::AnnotatedModule::moduleFromBitcode;
        } else if (optstr.compare("ll") == 0) {
            modFunc = whyr::AnnotatedModule::moduleFromIR;
        } else {
            std::cerr << "error: invalid option to " << options[INPUT_FORMAT].name << ": Unknown input format '" << optstr << "'" << std::endl;
            return 1;
        }
    }
    
    whyr::AnnotatedModule* mod;
    if (strncmp(input_file, "-", 2) == 0) {
        mod = modFunc(std::cin, "<stdin>", &settings);
    } else {
        std::ifstream file(input_file, std::ios::binary);
        mod = modFunc(file, input_file, &settings);
    }
    
    if (!mod) {
        std::cerr << "error: File could not be parsed" << std::endl;
        return 1;
    }
    
    mod->annotate();
    
    if (settings.rte) {
        addRTE(mod);
    }
    
    std::ostringstream out;
    generateWhy3(out, mod);
    
    if (options[OUTPUT]) {
        std::ofstream fout(options[OUTPUT].arg);
        fout << out.str();
        fout.flush();
    } else if (!options[PROVE]) {
        std::cout << out.str();
    }
    
    int exitCode = 0;
    
    for (std::list<whyr::whyr_warning>::iterator ii = settings.warnings.begin(); ii != settings.warnings.end(); ii++) {
        if (!settings.noWarn) {
            std::cerr << "warning: ";
            ii->printMessage(std::cerr);
        }
        if (settings.werror) exitCode = 1;
    }
    
    for (std::list<whyr::whyr_exception>::iterator ii = settings.errors.begin(); ii != settings.errors.end(); ii++) {
        std::cerr << "error: ";
        ii->printMessage(std::cerr);
        exitCode = 1;
    }
    
    if (options[PROVE]) {
        std::string pin = out.str();
        std::ostringstream pout;
        whyr::execWhy3(pin, pout, false, options[PROVER] ? options[PROVER].arg : whyr::PROVER_ALT_ERGO);
        
        whyr::Why3Output why3out(pout.str().c_str());
        if (why3out.error) {
            std::cerr << "error: in executing why3: " << why3out.message;
        } else {
            for (std::list<whyr::Why3Goal>::iterator ii = why3out.goals.begin(); ii != why3out.goals.end(); ii++) {
                std::cout << ii->goal << ": ";
                switch (ii->status) {
                    case whyr::Why3Goal::STATUS_VALID: {
                        std::cout << "VALID";
                        break;
                    }
                    case whyr::Why3Goal::STATUS_FAIL: {
                        std::cout << "FAILURE";
                        break;
                    }
                    case whyr::Why3Goal::STATUS_TIMEOUT: {
                        std::cout << "TIMEOUT";
                        break;
                    }
                    case whyr::Why3Goal::STATUS_UNKNOWN: {
                        std::cout << "UNKNOWN";
                        break;
                    }
                }
                std::cout << " (" << ii->time << "s)";
                if (ii->steps != -1) {
                    std::cout << " (" << ii->steps << " steps)";
                }
                std::cout << std::endl;
                
                if (ii->status == whyr::Why3Goal::STATUS_FAIL) {
                    std::cout << "=== Why3 stdout:" << std::endl;
                    std::cout << pout.str();
                    std::cout << "===" << std::endl;
                }
            }
        }
    }
    
    delete mod;
    return exitCode;
}
