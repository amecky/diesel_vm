#include "Tokenizer.h"
#include <cstdlib>
#include <stdio.h>
#include <windef.h>
#include <math.h>
#include "..\..\math\Vector.h"

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
/*
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
		case OP_NEW_VEC2: a = stack.pop(); b = stack.pop(); stack.push(Vector2f(a,b)); break;
	}
}
*/
Tokenizer::Tokenizer() {}

Tokenizer::Tokenizer(const Tokenizer& orig) {}

Tokenizer::~Tokenizer() {}

uint32 Tokenizer::convert(const char* p,ScriptContext& env,Token* tokens,int maxTokens,uint32* programmCounters) {
	int cnt = 0;
	int pcCounter = 0;
	programmCounters[pcCounter++] = 0;
	TypeDeclaration decl;
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
		else if ( ds::string::isCharacter(*p) ) {
			t = Token(Token::NAME,1.0f);
			const char* ident = p;
			while ( ds::string::isCharacter(*p) ) {
				++p;
			}
			t = env.findToken(ident,p-ident);
			if ( t.type == Token::DECLARATION ) {
				decl = env.getDeclaration(t.id);
			}
			else if ( t.type == Token::UNKNOWN && decl.type != DT_UNKNOWN ) {
				// register new variable
				printf("creating new variable type %d\n",decl.type);
				if ( decl.type == DT_VEC2 ) {
					Vector2f* v = new Vector2f;
					IdString hash = ds::string::murmur_hash(ident,p-ident,0);
					uint32 id = env.addVariable(hash,v);
					t = Token(Token::NAME,id);
				}
				else {
					t = Token(Token::NAME);
				}
			}
		}
		else {
			switch ( *p ) {
			case '(' : t = Token(Token::OPEN); break;
			case ')' : t = Token(Token::CLOSE); break;
			case ';' : {
				t = Token(Token::SEMICOLON); 
				programmCounters[pcCounter++] = cnt;
				//++pcCounter;
				break;
			}
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
	return pcCounter;
}

uint32 Tokenizer::parse(Token* tokens, int startToken,int endToken,const ScriptContext& env, Token* rpl) {
	uint32 cnt = 0;
	FunctionStackItem functionStack[32];
	int numFunctionStack = 0;
	int parLevel = 0;
	for ( int i = startToken; i < endToken; ++i ) {
		Token t = tokens[i];
		switch ( t.type ) {
		case Token::FLOAT : 
		case Token::NAME :
		case Token::DECLARATION :
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
	uint32 op = 0;
	for (uint32 i = 0; i < numTokens; ++i) {
		//Function f;
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
				//f = context.getFunction(t.id);
				op = BC_FUNCTION + t.id;
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
	VarType type = context.getVariableType(id);
	StackItem item = stack.pop();
	if ( type == DT_FLOAT ) {
		context.setVariable(id,item.values[0]);
	}
	else if ( type == DT_VEC2 ) {
		Vector2f v(item.values[0],item.values[1]);
		context.setVariable(id,v);
	}
}

bool Tokenizer::run(const uint32 *byteCode,ScriptContext& context,Stack &stack) {
	const uint32 *p = byteCode;
	while (true) {
		uint32 bc = *p++;
		uint32 op = bc_mask(bc);
		uint32 id = id_mask(bc);
		switch (op) {
			case BC_PUSH_VAR: {
				VarType type = context.getVariableType(id);
				if ( type == DT_VEC2 ) {
					stack.push(context.getVec2Variable(id));
				}
				else {
					stack.push(context.getVariable(id));
				}
				break;
			}
			case BC_NUMBER:
				stack.push(context.getData(id));
				break;
			case BC_CONSTANT:
				stack.push(context.getConstant(id));
				break;
			case BC_FUNCTION: {
				Function f = context.getFunction(id);
				if (  f.opCode == OP_ASSIGN ) {
					setAssignment(byteCode[0],stack,context);
				}
				else {
					f.functionPtr(stack);
				}
				break;
			}
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
uint32 Tokenizer::compile(const char* p, ScriptContext& context, std::vector<ScriptBlock>& blocks,bool debug) {
	Token tokens[256];
	uint32 programmCounter[32];
	uint32 lines = convert(p,context,tokens,256,programmCounter);
	if ( debug ) {
		printf("lines %d\n",lines);
		for ( int l = 0; l < lines; ++l ) {
			printf("%2d - pc %d\n",l,programmCounter[l]);
		}
		for ( int l = 0; l < lines - 1; ++l ) {
			printf("tokens detected %d\n",lines);
			for ( int i = programmCounter[l]; i < programmCounter[l+1]; ++i ) {
				if ( tokens[i].type != Token::FLOAT ) {
					printf("%2d : %s (%d)\n",i,context.translate(tokens[i]),tokens[i].id);  
				}
				else {
					printf("%2d : %s = %3.2f\n",i,context.translate(tokens[i]),tokens[i].value);  
				}
			}
		}
	}
	Token rpl[256];
	for ( int l = 0; l < lines - 1; ++l ) {
		uint32 rpls = parse(tokens,programmCounter[l],programmCounter[l+1],context,rpl);
		if ( debug ) {
			printf("-- RPL ------\n");
			printf("rpl tokens: %d\n",rpls);
			for ( int i = 0; i < rpls; ++i ) {
				if ( rpl[i].type == Token::FLOAT ) {
					printf("%2d : FLO %s %3.2f\n",i,context.translate(rpl[i]),context.getData(rpl[i].id));  
				}
				else if ( rpl[i].type == Token::FUNCTION ) {
					printf("%2d : FUNC %d\n",i,rpl[i].id);  
				}
				else {
					printf("%2d : %s %d\n",i,context.translate(rpl[i]),rpl[i].id);  
				}
			}
		}
		ScriptBlock block;
		block.bytes = generateBytecode(rpl,rpls,context,block.byteCode,64);
		blocks.push_back(block);
	}
		/*
	return generateBytecode(rpl,rpls,context,byteCode,maxCapacity);
	*/
	return 0;
}