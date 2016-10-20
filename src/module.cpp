/*
 * module.cpp
 *
 *  Created on: Sep 9, 2016
 *      Author: jrobbins
 */

#include <whyr/module.hpp>

namespace whyr {
    using namespace std;
    using namespace llvm;
    
    AnnotatedModule::AnnotatedModule(unique_ptr<Module>& llvm, WhyRSettings* settings) : settings{settings} {
        this->llvm = move(llvm);
    }
    
    AnnotatedModule::~AnnotatedModule() {
        for (list<AnnotatedFunction*>::iterator ii = functions.begin(); ii != functions.end(); ii++) {
            delete *ii;
        }
        
        LLVMContext* ctx = &llvm->getContext();
        llvm.release();
        delete ctx;
    }
    
    void AnnotatedModule::annotate() {
        // annotate all functions
        for (Module::iterator ii = llvm->begin(); ii != llvm->end(); ii++) {
            AnnotatedFunction* f = new AnnotatedFunction(this, &*ii);
            f->annotate();
            functions.push_back(f);
        }
    }
    
    list<AnnotatedFunction*>* AnnotatedModule::getFunctions() {
        return &functions;
    }
    
    Module* AnnotatedModule::rawIR() {
        // We have a UNIQUE pointer, not a normal pointer, to this module, so we have to alias it for you
        return &(*llvm);
    }
    
    AnnotatedFunction* AnnotatedModule::getFunction(const char* name) {
        for (list<AnnotatedFunction*>::iterator ii = functions.begin(); ii != functions.end(); ii++) {
            AnnotatedFunction* f = *ii;
            if (f->rawIR()->getName().compare(StringRef(name)) == 0) {
                return f;
            }
        }
        return NULL;
    }
    
    AnnotatedFunction* AnnotatedModule::getFunction(Function* func) {
        for (list<AnnotatedFunction*>::iterator ii = functions.begin(); ii != functions.end(); ii++) {
            AnnotatedFunction* f = *ii;
            if (f->rawIR() == func) {
                return f;
            }
        }
        return NULL;
    }
    
    WhyRSettings* AnnotatedModule::getSettings() {
        return settings;
    }
}
