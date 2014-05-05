/* 
 * File:   Tokenizer.h
 * Author: meckya
 *
 * Created on 25. April 2014, 11:09
 */

#ifndef TOKENIZER_H
#define	TOKENIZER_H
#include "ScriptVM.h"
#include <cstring>

struct FunctionStackItem {
    
    FunctionStackItem() {}
    FunctionStackItem(Token t, int p, int pl) : token(t), precedence(p), par_level(pl) {}

    inline int cmp(const FunctionStackItem &f) const {
            if (par_level != f.par_level) return par_level - f.par_level;
            return precedence - f.precedence;
    }
    inline bool operator<(const FunctionStackItem &other) const {return cmp(other) < 0;}
    inline bool operator<=(const FunctionStackItem &other) const {return cmp(other) <= 0;}
    inline bool operator==(const FunctionStackItem &other) const {return cmp(other) == 0;}
    inline bool operator>=(const FunctionStackItem &other) const {return cmp(other) >= 0;}
    inline bool operator>(const FunctionStackItem &other) const {return cmp(other) > 0;}
    
    Token token;
    int precedence;
    int par_level;

};

// -------------------------------------------------------
// Tokenizer
// -------------------------------------------------------
class Tokenizer {
    
public:
    Tokenizer();
    Tokenizer(const Tokenizer& orig);
    virtual ~Tokenizer();
    uint32 loadScript(const char* fileName,ScriptContext& context,std::vector<ScriptBlock>& blocks,bool debug = false);
    uint32 compile(const char* p,ScriptContext& context,std::vector<ScriptBlock>& blocks,bool debug = false);
    
    uint32 convert(const char* p,ScriptContext& env,Token* tokens,int maxTokens,uint32* programmCounters);

    uint32 parse(Token* tokens,int startToken,int endToken,const ScriptContext& env,Token* rpl);
    uint32 generateBytecode(Token* rpl,uint32 numTokens,const ScriptContext& context,uint32* byteCode,uint32 maxCapacity);
    bool run(const uint32 *byteCode,ScriptContext& context,Stack &stack,bool debug = false);
	bool run(std::vector<ScriptBlock>& blocks,ScriptContext& context,bool debug = false);
	void saveByteCode(std::vector<ScriptBlock>& blocks,ScriptContext& env);
	void loadByteCode(const char* fileName,std::vector<ScriptBlock>& blocks,ScriptContext& env);
private:
	const char* translateOpCode(uint32 opCode);
    void debugStack(const Stack& stack);
	void setAssignment(uint32 firstByte,Stack& stack,ScriptContext& context);	
};

#endif	/* TOKENIZER_H */
