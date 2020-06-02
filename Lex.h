
#ifndef LEX_H
#define LEX_H

namespace Lex {
    typedef unsigned char byte;

    enum Type {
        CONTROL, DATATYPE, GROUP, LITERAL, OPERATOR, ID
    };

    enum SubType {
        IF, ELSE, GOTO, RETURN, WHILE, SWITCH, CASE, DEFAULT, BREAK,
        UNSIGNED, INT, LONG, CHAR, FLOAT, BOOL, VOID, OPAREN, CPAREN, 
        COLON, OBRACK, CBRACK, COMMA, SEMICOLON, INT_LITERAL, FLT_LITERAL,
        CHAR_LITERAL, PLUS, MINUS, ASTK, DIV, MOD, NEG, DOT, GREATER, LESS,
        EQUAL, AMP, OR, CARET, ASSN, SHIFT, NAME
    };

    enum IOF {
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
        const char* pos;
        byte size;
        Type type;
        SubType subType;
        LitValue value;
    };

    class LexStream {
        private:
            Lex::Token* stream;
            Lex::Token* curr;
            Lex::Token* end;
        public:
            LexStream(int fileSize);
            ~LexStream();
            Lex::Token* allocate();
            void persist(const char* const file);      
    };  

    const char* const subtypeStrings[] = {"IF", "ELSE", "GOTO", "RETURN", 
        "WHILE", "SWITCH", "CASE", "DEFAULT", "BREAK", "UNSIGNED", "INT", 
        "LONG", "CHAR", "FLOAT", "BOOL", "VOID", "OPAREN", "CPAREN", "COLON", 
        "OBRACK", "CBRACK", "COMMA", "SEMICOLON", "INT_LITERAL", "FLT_LITERAL", 
        "CHAR_LITERAL", "PLUS", "MINUS", "ASTK", "DIV", "MOD", "NEG", "DOT", 
        "GREATER", "LESS", "EQUAL", "AMP", "OR", "CARET", "ASSN", "SHIFT", "NAME"};

    void lex(Lex::LexStream& tokens, char* const program); 
}

#endif