#include "Tokenizer.h"
#include <cstdlib>
#include <stdio.h>
#include <windef.h>
#include <math.h>
#include "..\..\math\Vector.h"
#include <fstream>
#include <io\BinaryWriter.h>
#include <io\BinaryLoader.h>

enum ByteCode {
	BC_FUNCTION    = 0x7f800000, // 2139095040
	BC_PUSH_VAR    = 0x7f900000, // 2140143616
	BC_NUMBER      = 0x7fa00000, // 2141192192
	BC_CONSTANT    = 0x7fc00000, 
	BC_ASSIGN      = 0x7fd00000, 
	BC_DECLARATION = 0x7fe00000, 
	BC_END         = 0x7fb00000  // 2142240768
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

// -------------------------------------------------------
// Tokenizer
// -------------------------------------------------------
Tokenizer::Tokenizer() {}

Tokenizer::Tokenizer(const Tokenizer& orig) {}

Tokenizer::~Tokenizer() {
}

// -------------------------------------------------------
// load script
// -------------------------------------------------------
uint32 Tokenizer::loadScript(const char* fileName,ScriptContext& context,std::vector<ScriptBlock>& blocks,bool debug) {
	std::string line;
	std::string result = "";
	std::ifstream myfile(fileName);
	if (myfile.is_open()) {
		while ( std::getline(myfile, line)) {
			result += line;
		}
		myfile.close();    
	}		
	return compile(result.c_str(),context,blocks,debug);
}

// -------------------------------------------------------
// Convert
// -------------------------------------------------------
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
				IdString hash = ds::string::murmur_hash(ident,p-ident,0);
				// register new variable
				//printf("creating new variable type %d\n",decl.type);
				if ( decl.type == DT_INT ) {
					int* v = new int;
					uint32 id = env.addVariable(hash,v);
					t = Token(Token::NAME,id);
				}
				else if ( decl.type == DT_VEC2 ) {
					Vector2f* v = new Vector2f;
					uint32 id = env.addVariable(hash,v);
					t = Token(Token::NAME,id);
				}
				else if ( decl.type == DT_FLOAT ) {
					float* v = new float;
					uint32 id = env.addVariable(hash,v);
					t = Token(Token::NAME,id);
				}
				else {
					t = Token(Token::NAME);
				}
				//printf("variable id %d\n",t.id);
			}
		}
		else {
			switch ( *p ) {
			case '(' : t = Token(Token::OPEN); break;
			case ')' : t = Token(Token::CLOSE); break;
			case ';' : {
				t = Token(Token::SEMICOLON); 
				programmCounters[pcCounter++] = cnt;
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
			case Token::DECLARATION :
				op = BC_DECLARATION;
				break;
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
		//printf("op %d id %d\n",op,t.id);
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
	else if ( type == DT_INT ) {
		context.setVariable(id,static_cast<int>(item.values[0]));
	}
	else if ( type == DT_VEC2 ) {
		Vector2f v(item.values[0],item.values[1]);
		context.setVariable(id,v);
	}
}

bool Tokenizer::run(const uint32 *byteCode,ScriptContext& context,Stack &stack,bool debug) {
	const uint32 *p = byteCode;
	while (true) {
		uint32 bc = *p++;
		uint32 op = bc_mask(bc);
		uint32 id = id_mask(bc);
		if ( debug ) {
			if ( op == BC_FUNCTION ) {
				printf("op %s id %s\n",translateOpCode(op),context.translateFunction(id));
			}
			else {
				printf("op %s id %d\n",translateOpCode(op),id);
			}
		}
		switch (op) {
			case BC_DECLARATION:
				break;
			case BC_PUSH_VAR: {
				VarType type = context.getVariableType(id);
				if ( type == DT_VEC2 ) {
					stack.push(context.getVec2Variable(id));
				}
				else if ( type == DT_INT ) {
					stack.push(context.getIntVariable(id));
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
					int first = 0;
					uint32 tmpOC = bc_mask(byteCode[0]);
					if ( tmpOC == BC_DECLARATION ) {
						++first;
					}
					setAssignment(byteCode[first],stack,context);
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

const char* Tokenizer::translateOpCode(uint32 opCode) {
	switch ( opCode ) {
		case 2139095040 : return "BC_FUNCTION";break;
		case 2140143616 : return "BC_PUSH_VAR";break;
		case 2141192192 : return "BC_NUMBER";break;
		case 2143289344 : return "BC_CONSTANT";break;
		case 2144337920 : return "BC_ASSIGN";break;
		case 2145386496 : return "BC_DECLARATION";break;
		case 2142240768 : return "BC_END";break;
		default: return "UNKNOWN";
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

bool Tokenizer::run(std::vector<ScriptBlock>& blocks,ScriptContext& context,bool debug) {
	Stack stack;
	for ( size_t i = 0; i < blocks.size(); ++i ) {
		ScriptBlock& block = blocks[i];
		if ( !run(block.byteCode,context,stack,debug) ) {
			return false;
		}
	}
	return true;
}

void Tokenizer::saveByteCode(std::vector<ScriptBlock>& blocks,ScriptContext& env ) {
	BinaryWriter writer;
	int signature[] = {0,8,15};
	writer.open("Test.sb",signature,3);
	uint32 vars = env.numVariables();
	for ( uint32 i = 0; i < vars; ++i ) {
		writer.startChunk(1,1);
		int varType = env.getVariableType(i);
		writer.write(varType);
		writer.write(env.getVariableHash(i));
		writer.closeChunk();
	}
	writer.startChunk(3,1);
	writer.write(env.numData());
	for ( uint32 i = 0; i < env.numData(); ++i ) {
		writer.write(env.getData(i));
	}
	writer.closeChunk();
	for ( size_t i = 0; i < blocks.size(); ++i ) {
		ScriptBlock& block = blocks[i];
		writer.startChunk(2,1);
		writer.write(block.bytes);
		writer.write(block.assignmentID);
		for ( int j = 0; j < block.bytes; ++j) {
			writer.write(block.byteCode[j]);
		}
		writer.closeChunk();
	}
	
	writer.close();
	
}

void Tokenizer::loadByteCode(const char* fileName,std::vector<ScriptBlock>& blocks,ScriptContext& env) {
	BinaryLoader loader;
	int signature[] = {0,8,15};
	loader.open(fileName,signature,3);
	while ( loader.openChunk() == 0 ) {
		if ( loader.getChunkID() == 1 ) {			
			int type = 0;
			loader.read(&type);
			IdString hash;
			loader.read(&hash);
			// enum VarType {DT_INT,DT_FLOAT,DT_VEC2,DT_VEC3,DT_COLOR,DT_UNKNOWN};
			if ( type == 0 ) {
				env.addVariable(hash,new int,true);
			}
			if ( type == 1 ) {
				env.addVariable(hash,new float,true);
			}
			if ( type == 2 ) {
				env.addVariable(hash,new Vector2f(0,0),true);
			}
			printf("adding var type %d = %d\n",type,hash);
		}
		else if ( loader.getChunkID() == 2 ) {							
			ScriptBlock block;
			loader.read(&block.bytes);
			loader.read(&block.assignmentID);
			for ( uint32 i = 0; i < block.bytes; ++i ) {
				loader.read(&block.byteCode[i]);
			}
			printf("block added with %d bytes\n",block.bytes);
			blocks.push_back(block);

		}
		else if ( loader.getChunkID() == 3 ) {	
			uint32 num = 0;
			loader.read(&num);
			printf("data %d\n",num);
			for ( uint32 i = 0; i < num; ++i ) {
				float v = 0.0f;
				loader.read(&v);
				env.setData(i,v);
			}
		}
		loader.closeChunk();
	}		
	loader.close();
}
