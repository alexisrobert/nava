cmake_minimum_required(VERSION 2.6)

include (${CMAKE_ROOT}/Modules/FindBISON.cmake)
include (${CMAKE_ROOT}/Modules/FindFLEX.cmake)

ADD_CUSTOM_TARGET(Parser echo "Creating parser.tab.c")

ADD_CUSTOM_COMMAND(
	SOURCE tokens.l
	COMMAND ${FLEX_EXECUTABLE}
	ARGS -o lexer.cpp tokens.l
	TARGET Parser
	OUTPUTS lexer.cpp)

ADD_CUSTOM_COMMAND(
		SOURCE parser.y
		COMMAND ${BISON_EXECUTABLE}
		ARGS -d parser.y -o parser.cpp
		TARGET Parser
		DEPENDS lexer.cpp
		OUTPUTS parser.cpp parser.hpp )

find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config DOC "llvm-config executable")

EXECUTE_PROCESS(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
	OUTPUT_VARIABLE LLVM_CXXFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

EXECUTE_PROCESS(
	COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
	OUTPUT_VARIABLE LLVM_LDFLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS} ${LLVM_LDFLAGS} -lLLVM-2.7 -g")

ADD_EXECUTABLE(nava main.cpp lexer.cpp parser.cpp ast/nodes.cpp ast/functions.cpp stdlib/stdlib.cpp)
