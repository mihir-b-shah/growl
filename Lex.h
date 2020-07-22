
#ifndef LEX_H
#define LEX_H

namespace Lex {
    
    enum class Type:char {
        CONTROL, DATATYPE, GROUP, LITERAL, OPERATOR, ID
    };

    enum class SubType:char {
        IF, ELSE, GOTO, RETURN, WHILE, SWITCH, CASE, DEFAULT, BREAK,
        UNSIGNED, INT, LONG, CHAR, FLOAT, BOOL, VOID, OPAREN, CPAREN, 
        COLON, OBRACK, CBRACK, COMMA, SEMICOLON, INT_LITERAL, FLT_LITERAL,
        CHAR_LITERAL, PLUS, MINUS, ASTK, DIV, MOD, NEG, DOT, GREATER, LESS,
        EQUAL, AMP, OR, CARET, ASSN, SHIFT, NAME
    };

    enum class IOF:char {
        UNDEFINED, PTRLVL, INT_VAL, FLOAT_VAL
    };

    // tagged union
    struct LitValue {
        IOF iof; // 0 if undefined, 1 if int, 2 if float. 
        union VHolder {
            int ptrLvl;
            long long ival;
            long double fval;
        } holder;
    };

    struct Token {
        typedef unsigned char byte;

        const char* pos;
        byte size;
        Type type;
        SubType subType;
        LitValue value;
    };

    class LexStream {
        private:
            Token* stream;
            Token* curr;
            Token* _end;
        public:
            LexStream(int fileSize);
            ~LexStream();
            Token* allocate();
            void persist(const char* const file);   
            Token* begin();
            Token* end();
    };  

    const char* const subtypeStrings[] = {"IF", "ELSE", "GOTO", "RETURN", 
        "WHILE", "SWITCH", "CASE", "DEFAULT", "BREAK", "UNSIGNED", "INT", 
        "LONG", "CHAR", "FLOAT", "BOOL", "VOID", "OPAREN", "CPAREN", "COLON", 
        "OBRACK", "CBRACK", "COMMA", "SEMICOLON", "INT_LITERAL", "FLT_LITERAL", 
        "CHAR_LITERAL", "PLUS", "MINUS", "ASTK", "DIV", "MOD", "NEG", "DOT", 
        "GREATER", "LESS", "EQUAL", "AMP", "OR", "CARET", "ASSN", "SHIFT", "NAME"};

    void lex(LexStream& tokens, int size, char* const program); 
}

#endif
