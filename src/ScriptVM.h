#pragma once
#include <cstring>
#include <dxstdafx.h>
#include <vector>
#include <utils\StringUtils.h>
#include "..\..\math\Vector.h"
#include <utils\Color.h>
// ---------------------------------------------------
// OpCode
// ---------------------------------------------------
enum OpCode {OP_ADD,OP_SUB,OP_MUL,OP_DIV,OP_SIN,OP_COS,OP_ASSIGN,OP_NEW_VEC2,OP_RND};

enum VarType {DT_INT,DT_FLOAT,DT_VEC2,DT_VEC3,DT_COLOR,DT_UNKNOWN};

struct TypeDeclaration {

	

	TypeDeclaration() : hash(0) , type(DT_UNKNOWN) {}
	TypeDeclaration(const char* name,VarType type) : type(type) {
		hash = ds::string::murmur_hash(name);
	}

	VarType type;
	uint32 hash;

};
// ---------------------------------------------------
// Token
// ---------------------------------------------------
struct Token {

	enum TokenType {UNKNOWN,OPEN,SEMICOLON,CLOSE,ASSIGN,SUB,ADD,DIV,MUL,NAME,FUNCTION,FLOAT,CONSTANT,DECLARATION};

	Token() : type(UNKNOWN) , id(0) {}      
	Token(TokenType type) : type(type) , id(0) {}
	Token(TokenType type,float value) : type(type) , value(value) {}
	Token(TokenType type,uint32 id) : type(type) , id(id) {}

	TokenType type;
	union {
		uint32 id;
		float value;
	};
};

// -------------------------------------------------------
// Stack item
// -------------------------------------------------------
struct StackItem {

	VarType type;
	float values[4];

	StackItem() : type(DT_FLOAT) {
		for ( int i = 0; i < 4; ++i ) {
			values[i] = 0.0f;
		}
	}

	StackItem(float v) : type(DT_FLOAT) {
		values[0] = v;
	}

	StackItem(const Vector2f& v) : type(DT_VEC2) {
		values[0] = v.x;
		values[1] = v.y;
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
	const StackItem get(uint32 id) const {
		return m_Data[id];
	}
	void push(float v) {
		assert( m_Size != m_Capacity );
		StackItem item(v);
		m_Data[m_Size++] = item;
	}
	void push(const Vector2f& v) {
		assert( m_Size != m_Capacity );
		StackItem item(v);
		m_Data[m_Size++] = item;
	}
	StackItem pop() {
		assert(m_Size > 0);
		return m_Data[--m_Size];
	}
	Vector2f popVec2() {
		StackItem item = pop();
		assert(item.type == DT_VEC2);
		return Vector2f(item.values[0],item.values[1]);
	}
	const uint32 size() const {
		return m_Size;
	}
private:
	StackItem m_Data[64];
	uint32 m_Size;
	uint32 m_Capacity;
};	

typedef void (*scriptFunctionPointer) (Stack& stack);

void scriptAddOperation(Stack& stack);
void scriptNewVec2(Stack& stack);
void scriptAssign(Stack& stack);
void scriptRandom(Stack& stack);
void scriptMul(Stack& stack);
// ---------------------------------------------------
// Function
// ---------------------------------------------------
struct Function {

	OpCode opCode;
	int precedence;
	int arity;
	uint32 hash;
	scriptFunctionPointer functionPtr;

	Function() {}
	Function(const char* name,OpCode opCode,int precedence,int arity,scriptFunctionPointer ptr) : opCode(opCode) , precedence(precedence) , arity(arity) , functionPtr(ptr) {
		hash = ds::string::murmur_hash(name,strlen(name),0);
	}

};

struct ScriptBlock {

	uint32 byteCode[64];
	uint32 bytes;
	uint32 assignmentID;

};
// ---------------------------------------------------
// Script context
// ---------------------------------------------------
class ScriptContext {

	struct FloatValue {

		uint32 hash;
		union {
			float* value;
			Vector2f* v2;
			Vector3f* v3;
			ds::Color* c;
		};
		VarType type;
		bool keepAlive;

		FloatValue() : hash(0) {}

		FloatValue(const char* name,float* v) : value(v) {
			hash = ds::string::murmur_hash(name,strlen(name),0);
			type = DT_FLOAT;
			keepAlive = false;
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
	uint32 addVariable(const char* name,float* value,bool keepAlive = false);
	uint32 addVariable(const char* name,Vector2f* v,bool keepAlive = false);
	uint32 addVariable(IdString hash,Vector2f* v,bool keepAlive = false);
	const VarType& getVariableType(uint32 id) const {
		return m_Variables[id].type;
	}
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
	const Vector2f getVec2Variable(uint32 idx) const {
		return *m_Variables[idx].v2;
	}
	void setVariable(uint32 idx,float v) {
		*m_Variables[idx].value = v;
	}
	void setVariable(uint32 idx,const Vector2f& v) {
		*m_Variables[idx].v2 = v;
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
	const TypeDeclaration& getDeclaration(uint32 id) const {
		return m_Declarations[id];
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