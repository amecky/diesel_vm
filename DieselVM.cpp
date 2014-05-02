//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
//#include "src\catch.hpp"
#include <cstdlib>
#include <stdio.h>
#include "src\ScriptVM.h"
#include "src\Tokenizer.h"

const bool SCRIPT_DEBUG = false;

int main(int argc, char** args) {
	ScriptContext context;
	float x = 100.0f;
	uint32 xid = context.addVariable("x",&x);
	Tokenizer tokenizer;
	uint32 byteCode[128];
	uint32 bytes = tokenizer.compile("x = 1 + 2 + sin(PI * 0.5)",context,byteCode,128,SCRIPT_DEBUG);
	if ( SCRIPT_DEBUG ) {
		printf("bytes: %d\n",bytes);
	}
	Stack stack;
	if ( tokenizer.run(byteCode,context,stack) ) {
		if ( SCRIPT_DEBUG ) {
			printf("stack size: %d\n",stack.size());
			for ( int i = 0; i < stack.size(); ++i ) {
				printf("%d = %3.2f\n",i,stack.get(i));
			}
		}
	}
	printf("x = %3.2f\n",x);
	return 0;
}

