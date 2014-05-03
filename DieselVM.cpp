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
	uint32 xid = context.addVariable("x",&x,true);
	Vector2f v(10,20);
	uint32 vidx = context.addVariable("v",&v);
	Tokenizer tokenizer;
	std::vector<ScriptBlock> blocks;

	//uint32 bytes = tokenizer.compile("x = 1 + 2 + sin(PI * 0.5)",context,byteCode,128,SCRIPT_DEBUG);
	uint32 bytes = tokenizer.compile("float y = 20 * x; x = 4 * y;v = v * x;",context,blocks,SCRIPT_DEBUG);
	if ( SCRIPT_DEBUG ) {
		printf("bytes: %d\n",bytes);
	}
	Stack stack;
	for ( size_t i = 0; i < blocks.size(); ++i ) {
		ScriptBlock& block = blocks[i];
		if ( tokenizer.run(block.byteCode,context,stack) ) {
			if ( SCRIPT_DEBUG ) {
				printf("stack size: %d\n",stack.size());
				for ( int i = 0; i < stack.size(); ++i ) {
					printf("%d = %3.2f\n",i,stack.get(i));
				}
			}
		}
	}
	printf("x = %3.2f \n",x);
	printf("v.x = %3.2f v.y = %3.2f\n",v.x,v.y);
	return 0;
}

