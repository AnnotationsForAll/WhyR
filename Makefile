##########################
# COMMON OPTIONS
##########################
INCLUDES = -I. -Iinclude
LIBS =  -pthread -lLLVMBitWriter -lLLVMBitReader -lLLVMCore -lLLVMSupport
OPTIONS = -std=c++11 -U__STRICT_ANSI__
H_FILES = $(wildcard include/whyr/*.hpp)
EXE_NAME = whyr

build: build_debug

##########################
# MAKEFILE CONFIGURATION
##########################
makefile.config: configure
	@echo "./configure"
	@ $(foreach v,$(shell echo '$(.VARIABLES)' | awk -v RS=' ' '/^[a-zA-Z0-9]+$$/'),$(v)='$($(v))') ./configure

-include makefile.config

##########################
# GOOGLE TEST REPOSITORY
##########################
GIT = git
GITFLAGS =

googletest:
	$(GIT) clone $(GITFLAGS) https://github.com/google/googletest.git

##########################
# OPTION PARSER
##########################
WGET = wget
WGETFLAGS = -q

optionparser.h:
	$(WGET) $(WGETFLAGS) "http://optionparser.sourceforge.net/optionparser.h" -O $@

##########################
# WAR PARSER
##########################
LEX = flex
LEXFLAGS =
YACC = bison
YACCFLAGS =

war_parser: src/war_lexer.hpp src/war_parser.hpp

src/war_lexer.hpp: src/war.l
	$(LEX) $(LEXFLAGS) -o $@ $<

src/war_parser.hpp: src/war.y
	$(YACC) $(YACCFLAGS) -o $@ $<

src/war.cpp: src/war_lexer.hpp src/war_parser.hpp
	@touch src/war.cpp

##########################
# DEBUG
##########################
DEBUG_C_FILES = $(wildcard src/*.cpp)
DEBUG_O_FILES = $(DEBUG_C_FILES:src/%.cpp=Debug/%.o)
DEBUG_OPTIONS = -O0 -g3 -Wall

build_debug: Debug/$(EXE_NAME)

Debug/$(EXE_NAME): Debug $(DEBUG_O_FILES)
	$(CXX) $(CXXFLAGS) $(OPTIONS) $(DEBUG_OPTIONS) $(DEBUG_O_FILES) -o $@ $(LIBS)

$(DEBUG_O_FILES): Debug/%.o: src/%.cpp $(H_FILES) optionparser.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTIONS) $(DEBUG_OPTIONS) -c $< -o $@

Debug:
	mkdir Debug

##########################
# RELEASE
##########################
RELEASE_C_FILES = $(wildcard src/*.cpp)
RELEASE_O_FILES = $(RELEASE_C_FILES:src/%.cpp=Release/%.o)
RELEASE_OPTIONS = -O3

build_release: Release/$(EXE_NAME)

Release$(EXE_NAME): Release $(RELEASE_O_FILES)
	$(CXX) $(CXXFLAGS) $(OPTIONS) $(RELEASE_OPTIONS) $(RELEASE_O_FILES) -o $@ $(LIBS)

$(RELEASE_O_FILES): Release/%.o: src/%.cpp $(H_FILES) optionparser.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTIONS) $(RELEASE_OPTIONS) -c $< -o $@

Release:
	mkdir Release

##########################
# LL -> BC FILES
##########################
LL_FILES = $(wildcard test/data/ir_files/*.ll)
BC_FILES = $(LL_FILES:test/data/ir_files/%.ll=test/data/bc_files/%.bc)

LLVMAS = llvm-as
LLVMASFLAGS = 

$(BC_FILES): test/data/bc_files/%.bc: test/data/ir_files/%.ll
	$(LLVMAS) $(LLVMASFLAGS) $< -o $@

test/data/bc_files:
	mkdir test/data/bc_files

bc_files: test/data/bc_files $(BC_FILES)

##########################
# TESTING
##########################
GTEST_C_FILES = googletest/googletest/src/gtest_main.cc googletest/googletest/src/gtest-all.cc
GTEST_O_FILES = $(GTEST_C_FILES:googletest/googletest/src/%.cc=Testing/%.o)
TEST_CASES_C_FILES = $(wildcard test/*.cpp)
TEST_CASES_O_FILES = $(TEST_CASES_C_FILES:test/%.cpp=Testing/%.o)
TESTING_C_FILES = $(filter-out src/main.cpp,$(wildcard src/*.cpp))
TESTING_O_FILES = $(TESTING_C_FILES:src/%.cpp=Testing/%.o)
TESTING_OPTIONS = -D_GNU_SOURCE -Igoogletest/googletest -isystem googletest/googletest/include/ -g
TEST_H_FILES = $(wildcard test/*.hpp)

test: build_test
	-./Testing/$(EXE_NAME)

build_test: Testing/$(EXE_NAME) $(BC_FILES)

Testing/$(EXE_NAME): Testing $(TESTING_O_FILES) $(TEST_CASES_O_FILES) $(GTEST_O_FILES)
	$(CXX) $(CXXFLAGS) $(OPTIONS) $(TESTING_OPTIONS) $(TESTING_O_FILES) $(TEST_CASES_O_FILES) $(GTEST_O_FILES) -o $@ $(LIBS)

$(TESTING_O_FILES): Testing/%.o: src/%.cpp $(H_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTIONS) $(TESTING_OPTIONS) -c $< -o $@

$(TEST_CASES_O_FILES): Testing/%.o: test/%.cpp $(H_FILES) $(TEST_H_FILES) $(GTEST_O_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTIONS) $(TESTING_OPTIONS) -c $< -o $@

$(GTEST_O_FILES): Testing/%.o: googletest/googletest/src/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTIONS) $(TESTING_OPTIONS) -c $< -o $@

$(GTEST_C_FILES): googletest

Testing:
	mkdir Testing

##########################
# MISC.
##########################
clean:
	$(RM) -r Debug
	$(RM) -r Testing
	$(RM) -r Release
	$(RM) $(BC_FILES)
	$(RM) makefile.config
	$(RM) src/war_lexer.hpp
	$(RM) src/war_parser.hpp

cleanall: clean
	$(RM) optionparser.h
	$(RM) -r googletest

.PHONY: build clean cleanall build_debug build_test test build_release bc_files war_parser