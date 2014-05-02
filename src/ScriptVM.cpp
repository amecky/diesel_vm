#include "ScriptVM.h"
#include <cstdlib>
#include <cstring>

ScriptContext::ScriptContext() : m_VariableIndex(0) , m_DataIndex(0) {
	m_Functions.push_back(Function("+",OP_ADD,12,2));
	m_Functions.push_back(Function("*",OP_MUL,16,2));
	m_Functions.push_back(Function("/",OP_DIV,16,2));
	m_Functions.push_back(Function("-",OP_SUB,12,2));
	m_Functions.push_back(Function("sin",OP_SIN,17,1));
	m_Functions.push_back(Function("cos",OP_COS,17,1));
	m_Functions.push_back(Function("=",OP_ASSIGN,1,1));

	m_Constants.push_back(ConstantValue("PI",3.14159265359f));

	m_Declarations.push_back(TypeDeclaration("int",TypeDeclaration::DT_INT));
	m_Declarations.push_back(TypeDeclaration("float",TypeDeclaration::DT_FLOAT));
	m_Declarations.push_back(TypeDeclaration("vec2",TypeDeclaration::DT_VEC2));
	m_Declarations.push_back(TypeDeclaration("vec3",TypeDeclaration::DT_VEC3));
	m_Declarations.push_back(TypeDeclaration("color",TypeDeclaration::DT_COLOR));
}

ScriptContext::ScriptContext(const ScriptContext& orig) {
}

ScriptContext::~ScriptContext() {
}

// -------------------------------------------------------
// Add variable
// -------------------------------------------------------
uint32 ScriptContext::addVariable(const char* name, float* value) {
	uint32 ret = m_VariableIndex;
	FloatValue* c = &m_Variables[m_VariableIndex];
	c->hash = ds::string::murmur_hash(name,strlen(name),0);
	c->value = value;
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
		case Token::FUNCTION : return "FUNCTION";break;
		case Token::FLOAT : return "FLOAT";break;
		case Token::NAME : return "NAME";break;
		case Token::DECLARATION : return "DECLARATION";break;
		case Token::ASSIGN: return "ASSIGN";break;
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
		case 2 : return "OP_DIV";
		case 3 : return "OP_SUB";
		case 4 : return "OP_SIN";
		case 5 : return "OP_COS";
		case 6 : return "OP_ASSIGN";
		default : return "UNKNOWN";
	}
}
