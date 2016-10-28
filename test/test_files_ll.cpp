/*
 * test_files.cpp
 *
 *  Created on: Sep 20, 2016
 *      Author: jrobbins
 */

#include "test_common.hpp"

#include <whyr/module.hpp>
#include <whyr/logic.hpp>
#include <whyr/esc_why3.hpp>
#include <whyr/exception.hpp>
#include <whyr/exec_why3.hpp>
#include <whyr/rte.hpp>

#include <list>
#include <string>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <string.h>

/**
 * The test class. Used to specify the format of the parameter.
 */
class FromLLToToWhy3WithoutErrorsTests : public ::testing::TestWithParam<const char*> {};

/**
 * The test function. Runs once for every bitcode file.
 */
TEST_P(FromLLToToWhy3WithoutErrorsTests,) {
    using namespace std;
    using namespace llvm;
    using namespace whyr;
    
    ASSERT_NO_THROW({
        try {
            // check a file can be loaded, annotated and generated without exception
            string fileLoc = string(GetParam());
            ifstream file = ifstream(fileLoc);
            AnnotatedModule* module = AnnotatedModule::moduleFromIR(file, fileLoc.c_str(), new WhyRSettings());
            ASSERT_TRUE(module);
            module->annotate();
            addRTE(module);
            ostringstream out;
            generateWhy3(out, module);
            // throw on both errors and warnings
            for (list<whyr::whyr_exception>::iterator ii = module->getSettings()->errors.begin(); ii != module->getSettings()->errors.end(); ii++) {
                throw *ii;
            }
            for (list<whyr::whyr_warning>::iterator ii = module->getSettings()->warnings.begin(); ii != module->getSettings()->warnings.end(); ii++) {
                throw *ii;
            }
            // check that the file generated runs through why3 correctly
            string why3 = out.str();
            ostringstream out2;
            execWhy3(why3, out2, true);
            string out_str = out2.str();
            Why3Output why3out(out_str.c_str());
            if (why3out.error) {
                string msg = to_string(why3out.line) + ":" + to_string(why3out.colBegin) + "-" + to_string(why3out.colEnd) + ":" + why3out.message;
                FAIL_WITH_MESSAGE(msg);
            }
            // clean up
            delete module;
        } catch (whyr_exception ex) {
            string errMsg = string("'") + ex.what() + "'";
            FAIL_WITH_MESSAGE(errMsg);
        }
    });
}

/**
 * Finds the names of all the files in the directory that stores all the bitcode files we want to test.
 */
static std::list<const char*> getFileNames() {
    std::list<const char*> a;
    
    {
        DIR* dir = opendir("test/data/ir_files");
        dirent* d = readdir(dir);
        while (d) {
            if (d->d_name[0] != '.' && strncmp(d->d_name, "fail", 4) != 0) { // ignore . and .., hidden files such as .svn, and failure tests
                a.push_back(strdup((std::string("test/data/ir_files/")+d->d_name).c_str()));
            }
            
            d = readdir(dir);
        }
        closedir(dir);
    }
    
    {
        DIR* dir = opendir("test/data/ir_files_nocompile");
        dirent* d = readdir(dir);
        while (d) {
            if (d->d_name[0] != '.' && strncmp(d->d_name, "fail", 4) != 0) { // ignore . and .., hidden files such as .svn, and failure tests
                a.push_back(strdup((std::string("test/data/ir_files_nocompile/")+d->d_name).c_str()));
            }
            
            d = readdir(dir);
        }
        closedir(dir);
    }
    
    return a;
}

/**
 * Create the new tests.
 */
INSTANTIATE_TEST_CASE_P(,FromLLToToWhy3WithoutErrorsTests,::testing::ValuesIn(getFileNames()));
