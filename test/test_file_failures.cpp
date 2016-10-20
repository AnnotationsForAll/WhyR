/*
 * test_file_failures.cpp
 *
 *  Created on: Oct 4, 2016
 *      Author: jrobbins
 */

#include "test_common.hpp"

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>

#include <list>
#include <string>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <string.h>

/**
 * The test class. Used to specify the format of the parameter.
 */
class ToWhy3WithErrorsTests : public ::testing::TestWithParam<const char*> {};

/**
 * The test function. Runs once for every bitcode file.
 */
TEST_P(ToWhy3WithErrorsTests,) {
    using namespace std;
    using namespace llvm;
    using namespace whyr;
    
    ASSERT_THROW({
        string fileLoc = string("test/data/bc_files/") + GetParam();
        ifstream file = ifstream(fileLoc);
        AnnotatedModule* module = AnnotatedModule::moduleFromBitcode(file, fileLoc.c_str(), new WhyRSettings());
        ASSERT_TRUE(module);
        module->annotate();
        ostringstream discarded;
        generateWhy3(discarded, module);
        for (list<whyr::whyr_exception>::iterator ii = module->getSettings()->errors.begin(); ii != module->getSettings()->errors.end(); ii++) {
            throw *ii;
        }
        delete module;
    }, whyr_exception);
}

/**
 * Finds the names of all the files in the directory that stores all the bitcode files we want to test.
 */
static std::list<const char*> getBitcodeFileNames() {
    std::list<const char*> a;
    
    DIR* dir = opendir("test/data/bc_files");
    dirent* d = readdir(dir);
    while (d) {
        if (d->d_name[0] != '.' && strncmp(d->d_name, "fail", 4) == 0) { // ignore . and .., hidden files such as .svn, and success tests
            a.push_back(strdup(d->d_name));
        }
        
        d = readdir(dir);
    }
    
    closedir(dir);
    return a;
}

/**
 * Create the new tests.
 */
INSTANTIATE_TEST_CASE_P(,ToWhy3WithErrorsTests,::testing::ValuesIn(getBitcodeFileNames()));
