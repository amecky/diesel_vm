#include "ScriptVM.h"
#include <cstdlib>
#include <cstring>
#include <math\GameMath.h>

void scriptAddOperation(Stack& stack) {
	assert(stack.size() >= 2);
	StackItem a = stack.pop();
	StackItem b = stack.pop();
	if ( a.type == DT_FLOAT && b.type == DT_FLOAT ) {
		stack.push(b.values[0] + a.values[0]);
	}
	else if ( a.type == DT_VEC2 && b.type == DT_VEC2 ) {
		Vector2f v(b.values[0],b.values[1]);
		Vector2f u(a.values[0],a.values[1]);
		stack.push(u+v);
	}
}

void scriptMul(Stack& stack) {
	assert(stack.size() >= 2);
	StackItem a = stack.pop();
	StackItem b = stack.pop();
	if ( a.type == DT_FLOAT && b.type == DT_FLOAT ) {
		stack.push(b.values[0] * a.values[0]);
	}
	else if ( a.type == DT_FLOAT && b.type == DT_VEC2 ) {
		Vector2f v(b.values[0],b.values[1]);
		float u = a.values[0];
		stack.push(v * u);
	}
	else if ( a.type == DT_VEC2 && b.type == DT_FLOAT) {
		Vector2f v(a.values[0],a.values[1]);
		float u = b.values[0];
		stack.push(v * u);
	}
}

void scriptNewVec2(Stack& stack) {
	assert(stack.size() >= 2);
	StackItem a = stack.pop();
	StackItem b = stack.pop();
	assert(a.type == DT_FLOAT);
	assert(b.type == DT_FLOAT);
	Vector2f v(b.values[0],a.values[0]);
	stack.push(v);
}

void scriptAssign(Stack& stack) {

}

void scriptPrint(Stack& stack) {
	StackItem a = stack.pop();
	if ( a.type == DT_FLOAT ) {
		printf("%3.2f\n",a.values[0]);
	}
	else if ( a.type == DT_INT ) {
		printf("%d\n",static_cast<int>(a.values[0]));
	}
	else if ( a.type == DT_VEC2 ) {
		printf("x : %3.2f y : %3.2f\n",a.values[0],a.values[1]);
	}
}

void scriptRandom(Stack& stack) {
	assert(stack.size() >= 2);
	StackItem a = stack.pop();
	StackItem b = stack.pop();
	assert(a.type == DT_FLOAT);
	assert(b.type == DT_FLOAT);
	float v = ds::math::random(b.values[0],a.values[0]);
	stack.push(v);
}

ScriptContext::ScriptContext() : m_VariableIndex(0) , m_DataIndex(0) {
	m_Functions.push_back(Function("+",OP_ADD,12,2,scriptAddOperation));
	m_Functions.push_back(Function("*",OP_MUL,16,2,scriptMul));
	//m_Functions.push_back(Function("/",OP_DIV,16,2));
	//m_Functions.push_back(Function("-",OP_SUB,12,2));
	//m_Functions.push_back(Function("sin",OP_SIN,17,1));
	//m_Functions.push_back(Function("cos",OP_COS,17,1));
	m_Functions.push_back(Function("=",OP_ASSIGN,1,1,scriptAssign));
	m_Functions.push_back(Function("random",OP_RND,20,2,scriptRandom));
	m_Functions.push_back(Function("print",OP_RND,19,1,scriptPrint));
	m_Functions.push_back(Function("vector2",OP_NEW_VEC2,20,2,scriptNewVec2));

	m_Constants.push_back(ConstantValue("PI",3.14159265359f));

	m_Declarations.push_back(TypeDeclaration("int",DT_INT));
	m_Declarations.push_back(TypeDeclaration("float",DT_FLOAT));
	m_Declarations.push_back(TypeDeclaration("vec2",DT_VEC2));
	m_Declarations.push_back(TypeDeclaration("vec3",DT_VEC3));
	m_Declarations.push_back(TypeDeclaration("color",DT_COLOR));
}

ScriptContext::ScriptContext(const ScriptContext& orig) {
}

ScriptContext::~ScriptContext() {
	for ( uint32 i = 0; i < m_VariableIndex; ++i ) {
		if ( !m_Variables[i].keepAlive ) {
			switch ( m_Variables[i].type) {
				case DT_INT: delete(m_Variables[i].ival);break;
				case DT_FLOAT: delete(m_Variables[i].value);break;
				case DT_VEC2: delete(m_Variables[i].v2);break;
			}
		}
	}
}

// -------------------------------------------------------
// Add variable
// -------------------------------------------------------
uint32 ScriptContext::addVariable(const char* name, float* value,bool keepAlive) {
	IdString hash = ds::string::murmur_hash(name,strlen(name),0);
	return addVariable(hash,value,keepAlive);
}

uint32 ScriptContext::addVariable(IdString hash,float* value,bool keepAlive) {
	uint32 ret = m_VariableIndex;
	FloatValue* c = &m_Variables[m_VariableIndex];
	c->hash = hash;
	c->value = value;
	*c->value = 0.0f;
	c->type = DT_FLOAT;
	c->keepAlive = keepAlive;
	++m_VariableIndex;
	return ret;
}

// -------------------------------------------------------
// Add variable
// -------------------------------------------------------
uint32 ScriptContext::addVariable(const char* name, Vector2f* value,bool keepAlive) {
	IdString hash = ds::string::murmur_hash(name,strlen(name),0);
	return addVariable(hash,value,keepAlive);
}

uint32 ScriptContext::connectVariable(const char* name,Vector2f* v,bool keepAlive) {
	IdString hash = ds::string::murmur_hash(name,strlen(name),0);
	for ( uint32 i = 0; i < numVariables(); ++i ) {
		FloatValue* c = &m_Variables[i];
		if ( c->hash == hash ) {
			delete c->v2;
			c->v2 = v;
			return i;
		}
	}
	return UINT_MAX;
}
// -------------------------------------------------------
// Add variable
// -------------------------------------------------------
uint32 ScriptContext::addVariable(IdString hash, Vector2f* value,bool keepAlive) {
	uint32 ret = m_VariableIndex;
	FloatValue* c = &m_Variables[m_VariableIndex];
	c->hash = hash;
	c->v2 = value;
	*c->v2 = Vector2f(0,0);
	c->keepAlive = keepAlive;
	c->type = DT_VEC2;
	++m_VariableIndex;
	return ret;
}

uint32 ScriptContext::addVariable(const char* name,int* v,bool keepAlive) {
	IdString hash = ds::string::murmur_hash(name,strlen(name),0);
	return addVariable(hash,v,keepAlive);
}
// -------------------------------------------------------
// Add variable
// -------------------------------------------------------
uint32 ScriptContext::addVariable(IdString hash, int* value,bool keepAlive) {
	uint32 ret = m_VariableIndex;
	FloatValue* c = &m_Variables[m_VariableIndex];
	c->hash = hash;
	c->ival = value;
	*c->ival = 0;
	c->keepAlive = keepAlive;
	c->type = DT_INT;
	++m_VariableIndex;
	return ret;
}
// -------------------------------------------------------
// Find token
// -------------------------------------------------------
Token ScriptContext::findToken(const char* s,int len) const {
	uint32 hash = ds::string::murmur_hash(s,len,0);
	for ( uint32 i = 0; i < m_Constants.size(); ++i ) {
		const ConstantValue& vf = m_Constants[i];
		if ( hash == vf.hash ) {
			return Token(Token::CONSTANT,i);
		}
	}
	for ( uint32 i = 0; i < m_Declarations.size(); ++i ) {
		const TypeDeclaration& f = m_Declarations[i];
		if ( f.hash == hash ) {
			return Token(Token::DECLARATION,i);
		}
	}
	for ( uint32 i = 0; i < m_Functions.size(); ++i ) {
		const Function& f = m_Functions[i];
		if ( f.hash == hash ) {
			return Token(Token::FUNCTION,i);
		}
	}
	for ( uint32 i = 0; i < m_VariableIndex; ++i ) {
		const FloatValue& f = m_Variables[i];
		if ( f.hash == hash ) {
			return Token(Token::NAME,i);
		}
	}
	return Token::UNKNOWN;
}

// -------------------------------------------------------
// Has function
// -------------------------------------------------------
const bool ScriptContext::hasFunction(const char* s, int len) const {
	uint32 hash = ds::string::murmur_hash(s,len,0);
	for ( uint32 i = 0; i < m_Functions.size(); ++i ) {
		const Function& f = m_Functions[i];
		if ( f.hash == hash ) {
			return true;
		}
	}
	return false;
}

// -------------------------------------------------------
// Translate token
// -------------------------------------------------------
const char* ScriptContext::translate(const Token& token) const {
	switch ( token.type ) {
		case Token::FUNCTION : 
			return "FUNCTION";break;
		case Token::FLOAT : 
			return "FLOAT";break;
		case Token::NAME : 
			return "NAME";break;
		case Token::DECLARATION : 
			return "DECLARATION";break;
		case Token::ASSIGN: 
			return "ASSIGN";break;
		case Token::SEMICOLON: 
			return "SEMICOLON";break;
		default : return "UNKNOWN";
	}
}

// -------------------------------------------------------
// Translate function
// -------------------------------------------------------
const char* ScriptContext::translateFunction(uint32 id) const {
	switch ( id ) {
		case 0 : return "OP_ADD";
		case 1 : return "OP_MUL";
		//case 2 : return "OP_DIV";
		//case 3 : return "OP_SUB";
		//case 4 : return "OP_SIN";
		//case 5 : return "OP_COS";
		case 2 : return "OP_ASSIGN";
		case 3 : return "OP_RANDOM";
		case 4 : return "OP_PRINT";
		case 5 : return "OP_NEW_VEC2";
		default : return "UNKNOWN";
	}
}
