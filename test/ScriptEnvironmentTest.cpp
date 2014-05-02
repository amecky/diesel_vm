#pragma comment(lib, "Diesel2D.lib")
#include <SDKDDKVer.h>
#include "..\src\catch.hpp"
#include "..\src\ScriptVM.h"
/*
TEST_CASE("Constructor","[ScriptEnvironment]") {
	ScriptContext env;
	REQUIRE( env.numFunctions() == 6 );
}

TEST_CASE("FindFunction","[ScriptEnvironment]") {
	ScriptContext env;
	uint32 idx = env.findFunction("*",1);
	REQUIRE( idx == 2 );
	idx = env.findFunction("sin",3);
	REQUIRE( idx == 4 );
	idx = env.findFunction("Hello",5);
	REQUIRE( idx == UINT_MAX );
}

TEST_CASE("FindToken","[ScriptEnvironment]") {
	ScriptContext env;
	Token t = env.findToken("*",1);
	REQUIRE( t.type == Token::FUNCTION );
	REQUIRE( t.id == 2 );
}

TEST_CASE("RegisterVariable","[ScriptEnvironment]") {
	ScriptContext env;
	env.addVariable("t",1.0f);
	env.addVariable("x",2.0f);
	REQUIRE( env.numVariables() == 2 );
	float t = env.getVariable("x");
	REQUIRE( t == 2.0f );
}
*/