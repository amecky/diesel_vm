#pragma once
#include <cstring>
#include <dxstdafx.h>
#include <vector>
#include <utils\StringUtils.h>
// ---------------------------------------------------
// OpCode
// ---------------------------------------------------
enum OpCode {OP_ADD,OP_SUB,OP_MUL,OP_DIV,OP_SIN,OP_COS,OP_ASSIGN};

struct TypeDeclaration {

	enum Type {DT_INT,DT_FLOAT,DT_VEC2,DT_VEC3,DT_COLOR};

	TypeDeclaration(const char* name,Type type) : type(type) {
		hash = ds::string::murmur_hash(name);
	}

	Type type;
	uint32 hash;

};
// ---------------------------------------------------
// Token
// ---------------------------------------------------
struct Token {

	enum TokenType {UNKNOWN,OPEN,CLOSE,ASSIGN,SUB,ADD,DIV,MUL,NAME,FUNCTION,FLOAT,CONSTANT,DECLARATION};

	Token() : type(UNKNOWN) {}      
	Token(TokenType type) : type(type) {}
	Token(TokenType type,float value) : type(type) , value(value) {}
	Token(TokenType type,uint32 id) : type(type) , id(id) {}

	TokenType type;
	union {
		uint32 id;
		float value;
	};
};

// ---------------------------------------------------
// Function
// ---------------------------------------------------
struct Function {

	OpCode opCode;
	int precedence;
	int arity;
	uint32 hash;

	Function() {}
	Function(const char* name,OpCode opCode,int precedence,int arity) : opCode(opCode) , precedence(precedence) , arity(arity) {
		hash = ds::string::murmur_hash(name,strlen(name),0);
	}

};

// ---------------------------------------------------
// Stack
// ---------------------------------------------------
class Stack {

public:
	Stack() : m_Size(0) , m_Capacity(64) {
		for ( int i = 0; i < 64; ++i ) {
			m_Data[i] = 0.0f;
		}
	}
	~Stack() {}
	const float get(uint32 id) const {
		return m_Data[id];
	}
	void push(float v) {
		if ( m_Size != m_Capacity ) {
			m_Data[m_Size++] = v;
		}
	}
	float pop() {
		// assert m_Size > 0
		return m_Data[--m_Size];
	}
	const uint32 size() const {
		return m_Size;
	}
private:
	float m_Data[64];
	uint32 m_Size;
	uint32 m_Capacity;
};	

// ---------------------------------------------------
// Script context
// ---------------------------------------------------
class ScriptContext {

	struct FloatValue {

		uint32 hash;
		float* value;

		FloatValue() : hash(0) {}

		FloatValue(const char* name,float* v) : value(v) {
			hash = ds::string::murmur_hash(name,strlen(name),0);
		}
	};    

	struct ConstantValue {

		uint32 hash;
		float value;

		ConstantValue() : hash(0) , value(0.0f) {}

		ConstantValue(const char* name,float v) : value(v) {
			hash = ds::string::murmur_hash(name,strlen(name),0);
		}
	};    

	typedef std::vector<Function> Functions;
	typedef std::vector<ConstantValue> Constants;
	typedef std::vector<TypeDeclaration> Declarations;

public:
	ScriptContext();
	ScriptContext(const ScriptContext& orig);
	virtual ~ScriptContext();
	uint32 addVariable(const char* name,float* value);
	const uint32 numConstants() const {
		return m_Constants.size();
	}
	const FloatValue* getVariables() const {
		return &m_Variables[0];
	}
	const ConstantValue* getConstants() const {
		return &m_Constants[0];
	}
	Token findToken(const char* s,int len) const;
	const char* translate(const Token& token) const;
	const Function& getFunction(uint32 id) const {
		return m_Functions[id];
	}
	const bool hasFunction(const char* s,int len) const;
	uint32 add(float value) {
		uint32 idx = m_DataIndex;
		m_Data[idx] = value;
		++m_DataIndex;
		return idx;
	}
	const float getVariable(uint32 idx) const {
		return *m_Variables[idx].value;
	}
	void setVariable(uint32 idx,float v) {
		*m_Variables[idx].value = v;
	}
	const float getData(uint32 idx) const {
		return m_Data[idx];
	}
	void setData(uint32 idx,float v) {
		m_Data[idx] = v;
	}
	const char* translateFunction(uint32 id) const;
	const float getConstant(uint32 id) const {
		return m_Constants[id].value;
	}
private:
	uint32 m_DataIndex;
	float m_Data[256];
	Constants m_Constants;
	FloatValue m_Variables[256];
	uint32 m_VariableIndex;
	Functions m_Functions;
	Declarations m_Declarations;
};