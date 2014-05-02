#include "..\src\catch.hpp"
#include "..\src\Tokenizer.h"
/*
TEST_CASE("Basicparse","[Tokenizer]") {
	Tokenizer tokenizer;	
	Token tokens[32];
	ScriptContext env;
	uint32 numTokens = tokenizer.convert("1 + 2 + 3 * 4",env,tokens,32);
	REQUIRE(numTokens == 7);
	REQUIRE(tokens[0].type == Token::NUMBER);
	REQUIRE(tokens[0].value == 1.0f);
	REQUIRE(tokens[1].type == Token::FUNCTION);
	REQUIRE(tokens[1].id == 0);
}
*/
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