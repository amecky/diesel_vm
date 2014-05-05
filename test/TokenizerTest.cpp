#include "..\src\catch.hpp"
#include "..\src\Tokenizer.h"
/*
TEST_CASE("Basicparse","[Tokenizer]") {
	Tokenizer tokenizer;	
	Token tokens[32];
	ScriptContext env;
	uint32 pcCounter[16];
	uint32 lines = tokenizer.convert("1 + 2 + 3 * 4;",env,tokens,32,pcCounter);
	REQUIRE(lines == 2);
	REQUIRE(pcCounter[0] == 0);
	REQUIRE(pcCounter[1] == 7);
	REQUIRE(tokens[0].type == Token::FLOAT);
	REQUIRE(tokens[1].type == Token::FUNCTION);
	REQUIRE(tokens[1].id == 0);
}

TEST_CASE("ParseTwoLines","[Tokenizer]") {
	Tokenizer tokenizer;	
	Token tokens[32];
	ScriptContext env;
	uint32 pcCounter[16];
	uint32 lines = tokenizer.convert("1 + 2 + 3 * 4;2 - 3;",env,tokens,32,pcCounter);
	REQUIRE(lines == 3);
	REQUIRE(pcCounter[0] == 0);
	REQUIRE(pcCounter[1] == 7);
	REQUIRE(pcCounter[2] == 10);
}

TEST_CASE("ParseIntVar","[Tokenizer]") {
	Tokenizer tokenizer;	
	Token tokens[32];
	ScriptContext env;
	uint32 pcCounter[16];
	uint32 lines = tokenizer.convert("int iv = 1 + 2 + 3 * 4;2 - 3;",env,tokens,32,pcCounter);
	REQUIRE(lines == 3);
	REQUIRE(pcCounter[0] == 0);
	REQUIRE(pcCounter[1] == 10);
	REQUIRE(pcCounter[2] == 13);
	REQUIRE(env.numVariables() == 1);
	VarType type = env.getVariableType(0);
	REQUIRE(type == DT_INT);
	REQUIRE(env.getIntVariable(0) == 0);
}
TEST_CASE("ParseSeveralVars","[Tokenizer]") {
	Tokenizer tokenizer;	
	Token tokens[32];
	ScriptContext env;
	uint32 pcCounter[16];
	uint32 lines = tokenizer.convert("int iv = 1;float fv = 2.0;vec2 ve = vector2(1,2);",env,tokens,32,pcCounter);
	REQUIRE(lines == 4);
	REQUIRE(env.numVariables() == 3);
	VarType type = env.getVariableType(0);
	REQUIRE(type == DT_INT);
	REQUIRE(env.getIntVariable(0) == 0);
	type = env.getVariableType(1);
	REQUIRE(type == DT_FLOAT);
	REQUIRE(env.getVariable(1) == 0.0f);
	type = env.getVariableType(2);
	REQUIRE(type == DT_VEC2);
	Vector2f v = env.getVec2Variable(2);
	REQUIRE(v.x == 0.0f);
	REQUIRE(v.y == 0.0f);
}
*/
/*
TEST_CASE("RuneOne","[Tokenizer]") {
	Tokenizer tokenizer;	
	ScriptContext env;
	int x = 0;
	env.addVariable("x",&x,true);
	std::vector<ScriptBlock> blocks;
	uint32 numTokens = tokenizer.compile("x = 1 + 2 + 3 * 4;",env,blocks);
	tokenizer.run(blocks,env);
	REQUIRE(x == 15);
}
*/
/*
TEST_CASE("RuneTwo","[Tokenizer]") {
	Tokenizer tokenizer;	
	ScriptContext env;
	Vector2f v;
	env.addVariable("v",&v,true);
	std::vector<ScriptBlock> blocks;
	uint32 numTokens = tokenizer.compile("vec2 t = vector2(2,1);v = t + vector2(4,5);",env,blocks,true);
	tokenizer.run(blocks,env,true);
	REQUIRE(v.x == 6.0f);
	REQUIRE(v.y == 6.0f);
}
*/
/*
TEST_CASE("RuneThree","[Tokenizer]") {
	Tokenizer tokenizer;	
	ScriptContext env;
	Vector2f v;
	env.addVariable("v",&v,true);
	std::vector<ScriptBlock> blocks;
	tokenizer.loadScript("Test.script",env,blocks,true);
	tokenizer.saveByteCode(blocks,env);
	tokenizer.run(blocks,env,true);
	REQUIRE(v.x == 60.0f);
	REQUIRE(v.y == 90.0f);
}
*/
TEST_CASE("RuneFour","[Tokenizer]") {
	Tokenizer tokenizer;	
	ScriptContext env;
	Vector2f v;
	std::vector<ScriptBlock> blocks;
	tokenizer.loadByteCode("Test.sb",blocks,env);
	uint32 id = env.connectVariable("v",&v,true);
	printf("connected to %d\n",id);
	tokenizer.run(blocks,env,true);
	REQUIRE(v.x == 60.0f);
	REQUIRE(v.y == 90.0f);
}
/*
TEST_CASE("GenerateBytecode","[Tokenizer]") {
	Tokenizer tokenizer;	
	uint32 byteCode[64];
	ScriptEnvironment env;
	uint32 numTokens = tokenizer.compile("1 + 2 + 3 * 4",env,byteCode,64);
	REQUIRE(numTokens == 8);
}

TEST_CASE("Run","[Tokenizer]") {
	Tokenizer tokenizer;	
	uint32 byteCode[64];
	ScriptEnvironment env;
	uint32 numTokens = tokenizer.convert("1 + 2 + 3 * 4",env,byteCode,64);
	REQUIRE(numTokens == 8);
	float data[32];
	Stack stack(data,32);
	tokenizer.run(byteCode,stack,data);
	REQUIRE(stack.size == 1);
	REQUIRE(data[0] == 15.0f);
}
*/
/*
TEST_CASE("RunSine","[Tokenizer]") {
	Tokenizer tokenizer;	
	uint32 byteCode[64];
	ScriptContext env;
	env.addVariable("x",10.0f);
	uint32 numTokens = tokenizer.compile("cos(3.14) * 0.5 + 2 + 3 * 4",env,byteCode,64);
	//REQUIRE(numTokens == 12);
	float data[32];
	Stack stack;
	if ( tokenizer.run(byteCode,env,stack) ) {
		printf("stack size: %d\n",stack.size());
		for ( int i = 0; i < stack.size(); ++i ) {
			printf("%d = %3.2f\n",i,stack.get(i));
		}
	}
	else {
		printf("ERROR\n");
	}
	//REQUIRE(stack.size == 2);
	//REQUIRE(env.getVariable("x") == 100.0f);
}
*/