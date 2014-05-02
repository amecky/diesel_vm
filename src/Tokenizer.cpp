#include "Tokenizer.h"
#include <cstdlib>
#include <stdio.h>
#include <windef.h>
#include <math.h>

enum ByteCode {
	BC_FUNCTION = 0x7f800000, // 2139095040
	BC_PUSH_VAR = 0x7f900000, // 2140143616
	BC_NUMBER   = 0x7fa00000, // 2141192192
	BC_CONSTANT = 0x7fc00000, 
	BC_ASSIGN   = 0x7fd00000, 
	BC_END      = 0x7fb00000  // 2142240768
};

/// Returns the byte code operation part of the byte code word.
static inline uint32 bc_mask(uint32 i) {
	return i & 0xfff00000;
}

/// Returns the id part of the byte code word.
static inline uint32 id_mask(uint32 i) {
	return i & 0x000fffff;
}

/// Returns true if the byte code word is a BC_PUSH_FLOAT operation.
static inline bool is_bc_push_float(uint32 i) {
	return (i & 0x7f80000) != 0x7f8;
}

/// Computes the function specified by @a op_code on the @a stack.
static inline void compute_function(OpCode op_code, Stack &stack) {
	float a,b;
	switch(op_code) {
	case OP_ADD: 
		b = stack.pop(); 
		a = stack.pop(); 
		stack.push(a+b); 
		break;
	case OP_SUB: b=stack.pop(); a=stack.pop(); stack.push(a-b); break;
	case OP_MUL: b=stack.pop(); a=stack.pop(); stack.push(a*b); break;
	case OP_DIV: b=stack.pop(); a=stack.pop(); stack.push(a/b); break;
	case OP_SIN: stack.push(sinf(stack.pop())); break;
	case OP_COS: stack.push(cosf(stack.pop())); break;
	}
}

Tokenizer::Tokenizer() {}

Tokenizer::Tokenizer(const Tokenizer& orig) {}

Tokenizer::~Tokenizer() {}

int Tokenizer::convert(const char* p,ScriptContext& env,Token* tokens,int maxTokens) {
	int cnt = 0;
	while ( *p != 0 ) {    
		Token t;
		// Numbers
		if ( *p >= '0' && *p <= '9' ) {
			char* out;
			float value = ds::string::strtof(p,&out);            
			p = out;
			uint32 id = env.add(value);
			t = Token(Token::FLOAT,id);
		}
		else if ( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || ( *p == '_' ) || ds::string::isDigit(*p) ) {
			t = Token(Token::NAME,1.0f);
			const char* ident = p;
			while ( (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || ( *p == '_' ) || ds::string::isDigit(*p) ) {
				++p;
			}
			//int id = env.findFunction(ident,p-ident);
			t = env.findToken(ident,p-ident);
			if ( t.type == Token::UNKNOWN ) {
				t = Token(Token::NAME);
			}
		}
		else {
			switch ( *p ) {
			case '(' : t = Token(Token::OPEN); break;
			case ')' : t = Token(Token::CLOSE); break;
			case ' ' : 
			case '\t' : 
			case '\r' : 
			case '\n' : 
				break;
			default:
				char s1[2] = {*p,0};
				char s2[3] = {*p, *(p+1), 0};
				if (s2[1] && env.hasFunction(s2,2)) {
					t = env.findToken(s2,2);
					++p;
				} else {
					t = env.findToken(s1,1);
				}
				break;
			}
			++p;
		}
		//printf("%d = %s\n",cnt,env.translate(t));
		if ( t.type != Token::UNKNOWN ) {
			if ( cnt < maxTokens ) {
				tokens[cnt++] = t;
			}
		}
	}
	return cnt;
}

int Tokenizer::parse(Token* tokens, int numTokens,const ScriptContext& env, Token* rpl) {
	int cnt = 0;
	FunctionStackItem functionStack[32];
	int numFunctionStack = 0;
	int parLevel = 0;
	for ( int i = 0; i < numTokens; ++i ) {
		Token t = tokens[i];
		switch ( t.type ) {
		case Token::FLOAT : 
		case Token::NAME :
		case Token::CONSTANT:
			rpl[cnt++] = t;
			break;
		case Token::OPEN :
			++parLevel;
			break;
		case Token::CLOSE :
			--parLevel;
			break;
		case Token::FUNCTION :
			FunctionStackItem f(t, env.getFunction(t.id).precedence, parLevel);
			while (numFunctionStack > 0 && functionStack[numFunctionStack-1] >= f) {
				rpl[cnt++] = functionStack[--numFunctionStack].token;
			}
			functionStack[numFunctionStack++] = f;
			break;
		}
	}
	while (numFunctionStack > 0 ) {
		rpl[cnt++] = functionStack[--numFunctionStack].token;
	}
	if ( parLevel != 0 ) {
		printf("Error: unmatching paranthesis detected");
		return -1;
	}
	return cnt;
}

uint32 Tokenizer::generateBytecode(Token* rpl, uint32 numTokens, const ScriptContext& context, uint32* byteCode, uint32 maxCapacity) {
	uint32 size = 0;
	uint32 op;
	for (uint32 i = 0; i < numTokens; ++i) {
		Function f;
		Token t = rpl[i];
		switch (t.type) {
			case Token::FLOAT :
				op = BC_NUMBER + t.id;                
				break;
			case Token::CONSTANT :
				op = BC_CONSTANT + t.id;                
				break;
			case Token::NAME:
				op = BC_PUSH_VAR + t.id;
				break;
			case Token::FUNCTION:
				f = context.getFunction(t.id);
				op = BC_FUNCTION + f.opCode;
				break;
			default:
				printf("Unknown token\n");
				break;
		}
		if ( size < maxCapacity ) {
			byteCode[size++] = op;
		}
	}
	op = BC_END;
	if (size < maxCapacity) {
		byteCode[size++] = op;
	}
	return size;
}

void Tokenizer::setAssignment(uint32 firstByte,Stack& stack,ScriptContext& context) {
	uint32 op = bc_mask(firstByte);
	uint32 id = id_mask(firstByte);
	float v = stack.pop();
	context.setVariable(id,v);
}

bool Tokenizer::run(const uint32 *byteCode,ScriptContext& context,Stack &stack) {
	const uint32 *p = byteCode;
	while (true) {
		uint32 bc = *p++;
		uint32 op = bc_mask(bc);
		uint32 id = id_mask(bc);
		switch (op) {
			case BC_PUSH_VAR:
				stack.push(context.getVariable(id));
				break;
			case BC_NUMBER:
				stack.push(context.getData(id));
				break;
			case BC_CONSTANT:
				stack.push(context.getConstant(id));
				break;
			case BC_FUNCTION:
				//printf("function %d %s\n",id,context.translateFunction(id));
				if ( (OpCode)id == OP_ASSIGN ) {
					setAssignment(byteCode[0],stack,context);
				}
				else {
					compute_function((OpCode)id, stack);
				}
				break;
			case BC_END:
				return true;
			default: // BC_PUSH_FLOAT
				stack.push(context.getData(bc));
				break;
		}
	}
}

void Tokenizer::debugStack(const Stack& stack) {
	printf("stack size: %d\n",stack.size());
	for ( int i = 0; i < stack.size(); ++i ) {
		printf("%d = %3.2f\n",i,stack.get(i));
	}
}

// --------------------------------------------------------
// compile
// --------------------------------------------------------
uint32 Tokenizer::compile(const char* p, ScriptContext& context, uint32* byteCode, uint32 maxCapacity,bool debug) {
	Token tokens[256];
	int total = convert(p,context,tokens,256);
	if ( debug ) {
		printf("tokens detected %d\n",total);
		for ( int i = 0; i < total; ++i ) {
			if ( tokens[i].type != Token::FLOAT ) {
				printf("%d : %s (%d)\n",i,context.translate(tokens[i]),tokens[i].id);  
			}
			else {
				printf("%d : %s = %3.2f\n",i,context.translate(tokens[i]),tokens[i].value);  
			}
		}
	}
	Token rpl[256];
	int rpls = parse(tokens,total,context,rpl);
	if ( debug ) {
		printf("-- RPL ------\n");
		printf("rpl tokens: %d\n",rpls);
		for ( int i = 0; i < rpls; ++i ) {
			if ( rpl[i].type == Token::FLOAT ) {
				printf("%d : FLO %s %3.2f\n",i,context.translate(rpl[i]),context.getData(rpl[i].id));  
			}
			else if ( rpl[i].type == Token::FUNCTION ) {
				printf("%d : FUNC %d\n",i,rpl[i].id);  
			}
			else {
				printf("%d : %s %d\n",i,context.translate(rpl[i]),rpl[i].id);  
			}
		}
	}
	return generateBytecode(rpl,rpls,context,byteCode,maxCapacity);
}