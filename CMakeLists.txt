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

ADD_EXECUTABLE(nava main.cpp lexer.cpp parser.cpp )
