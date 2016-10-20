/*
 * import.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;

    AnnotatedModule* AnnotatedModule::moduleFromBitcode(istream& file, const char* file_name, WhyRSettings* settings) {
        vector<char> buffer((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));
        char a[buffer.size()];
        copy(buffer.begin(), buffer.end(), a);
        
        if (file.fail()) { // if there was a read error, dson't even try passing in what we got to LLVM
            return NULL;
        }

        LLVMContext* ctx = new LLVMContext();
        MemoryBufferRef buf(StringRef(a, buffer.size()), StringRef(string(file_name)));
        ErrorOr<unique_ptr<Module>> m = parseBitcodeFile(buf, *ctx);
        
        // Note that parseBitcodeFile actually throws an exception on a parse error, making it so m is never an error, only a correct value.
        // FIXME: Make this function catch this exception LLVM throws
        
        if (m) {
            return new AnnotatedModule(*m, settings);
        } else {
            return NULL;
        }
    }
    
    AnnotatedModule* AnnotatedModule::moduleFromIR(istream& file, const char* file_name, WhyRSettings* settings) {
        //TODO
        return NULL;
    }
}
