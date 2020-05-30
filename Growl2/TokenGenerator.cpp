 
#include <cstring>
#include <cctype>
#include "Global.h"

/**
 * Defines a set of keywords. copied from my deformed java code.
 * This is bare bones
 * 
 * Control: if, else, goto, return, while, switch, case, default, break (done!)
 * Data types/modifier: user-def type, int, long, char, float, void, and pointers. (done!)
 * Group: (, ), :, {, }, ',', ';' (done!)
 * Literals: number, char, floating-pt. (done!)
 * Operators: +, -, /, %, ~, ., *, <, >, &, |, ^, = (done!)
 * Identifier: letter1/underscore, then any number too. no $. (done!)
 * 
 * Hardcoded token generator.
 */

typedef unsigned char byte;

enum Type {
    CONTROL, DATATYPE, GROUP, LITERAL, OPERATOR, ID
};

enum SubType {
    IF, ELSE, GOTO, RETURN, WHILE, SWITCH, CASE, DEFAULT, BREAK,
    UNSIGNED, INT, LONG, CHAR, FLOAT, BOOL, VOID, OPAREN, CPAREN, 
    COLON, OBRACK, CBRACK, COMMA, SEMICOLON, INT_LITERAL, FLT_LITERAL,
    CHAR_LITERAL, PLUS, MINUS, ASTK, DIV, MOD, NEG, DOT, GREATER, LESS,
    AMP, OR, CARET, ASSN
};

enum IOF {
    UNDEFINED, PTRLVL, INT, FLOAT
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

static inline bool validLetter(const char letter) {
    return std::isalpha(letter) || letter == '_';
}

/*  parse an identifier or int/flt litera
    right now... only a decimal int literal of arbitrary size
    and a float of arbitrary size. no suffixes either.
    all of this will be handled by a regex enabled preprocessor. */
static inline char* const parseWord(Token* base, char* const data) {
    if(validLetter(data[0])) {
        // identifier
        int ct = 0;
        // if u actually have a 255 character identifier.... something is very wrong
        // also hopefully this gets vectorized or unrolled....
        while(__builtin_expect(ct < 255 && !isspace(data[ct]), true)) {
            ++ct;
        }
        if(ct == 255) {
            throw Global::InvalidLiteral;
        }
        return data+ct;
    } else if(std::isdigit(data[0])) {
        // int/float
        int ct = 0;
        while(ct < 255 && isdigit(data[ct]) && data[ct] != '.') {
            ++ct;
        }
        if(ct == 255 || data[ct] != '.' && !isspace(data[ct])) {
            throw Global::InvalidLiteral;
        } else if(data[ct] == '.') {
            // use a float 
            ++ct;
            char* dotPtr = data+ct;
            while(ct < 255 && isdigit(data[ct])) {
                ++ct;
            }
            if(ct == 255 || !isspace(data[ct])) {
                throw Global::InvalidLiteral;
            }
            long double res = 0;
            char* readPtr = data+ct-1;
            while(readPtr != data) {
                res *= 10;
                res += *readPtr;
                --readPtr;
            }
            readPtr = data+ct;
            long double power = 0.1;
            while(dotPtr != readPtr) {
                res += power*(*dotPtr);
                ++dotPtr;
                power *= 0.1;
            }
            base->value.iof = IOF::FLOAT;
            base->value.holder.ival = res;
            return dotPtr;
        } else {
            // use an int, num ranges from [data, data+ct).
            char* readPtr = data+ct-1;
            long long res = 0;
            char* ret = readPtr+1;
            while(readPtr != data) {
                res *= 10;
                res += *readPtr;
                --readPtr;
            }
            base->value.iof = IOF::INT;
            base->value.holder.ival = res;
            return ret;
        }
    } else {
        throw Global::InvalidIdentifier;
    }
}

static inline bool checkEnd(const char data) {
    return std::isspace(data) || data == '\0';
}

static inline int scanPtrLvl(char* const ptr) {
    int ct = 0;
    while(*(ptr+ct) == '*') {
        ++ct;
    }
    return ct;
}

static inline char* const parseCharLiteral(LitValue::VHolder* holder, char* const ptr) {
    if(*(ptr+2) == '\'') {
        // a proper character
        holder->ival = *(ptr+1);
        return ptr+3;
    } else if(*(ptr+1) == '\\' && *(ptr+3) == '\'') {
        // same as java, escape sequences.
        switch(*(ptr+2)) {
            case 't':
                holder->ival = '\t';
            case 'b':
                holder->ival = '\b';
            case 'n':
                holder->ival = '\n';
            case 'r':
                holder->ival = '\r';
            case 'f':
                holder->ival = '\t';
            case '\'':
                holder->ival = '\'';
            case '\"':
                holder->ival = '\"';
            case '\\':
                holder->ival = '\\';
            default:
                throw Global::InvalidEscapeSequence;
        }
    } else {
        throw Global::InvalidCharacter;
    }
}

static inline char* const parse(Token* base, char* const data) {
    base->size = 1;
    base->pos = data;

    switch(data[0]) {
        case '(':
            base->type = Type::GROUP;
            base->subType = SubType::OPAREN;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case ')':
            base->type = Type::GROUP;
            base->subType = SubType::CPAREN;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case ':':
            base->type = Type::GROUP;
            base->subType = SubType::COLON;
			base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '{':
            base->type = Type::GROUP;
            base->subType = SubType::OBRACK;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '}':
            base->type = Type::GROUP;
            base->subType = SubType::CBRACK;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case ',':
            base->type = Type::GROUP;
            base->subType = SubType::COMMA;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case ';':
            base->type = Type::GROUP;
            base->subType = SubType::SEMICOLON;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '+':
            base->type = Type::OPERATOR;
            base->subType = SubType::PLUS;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '-':
            base->type = Type::OPERATOR;
            base->subType = SubType::MINUS;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '*':
            base->type = Type::OPERATOR;
            base->subType = SubType::ASTK;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '/':
            base->type = Type::OPERATOR;
            base->subType = SubType::DIV;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '%':
            base->type = Type::OPERATOR;
            base->subType = SubType::MOD;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '~':
            base->type = Type::OPERATOR;
            base->subType = SubType::NEG;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '.':
            base->type = Type::OPERATOR;
            base->subType = SubType::DOT;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '<':
            base->type = Type::OPERATOR;
            base->subType = SubType::LESS;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '>':
            base->type = Type::OPERATOR;
            base->subType = SubType::GREATER;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '&':
            base->type = Type::OPERATOR;
            base->subType = SubType::AMP;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '|':
            base->type = Type::OPERATOR;
            base->subType = SubType::OR;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '^':
            base->type = Type::OPERATOR;
            base->subType = SubType::CARET;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '=':
            base->type = Type::OPERATOR;
            base->subType = SubType::ASSN;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '\'':
            // char literal
            base->type = Type::LITERAL;
            base->subType = SubType::CHAR_LITERAL;
            base->value.iof = IOF::INT;
            return parseCharLiteral(&(base->value.holder), data);
        // for all keywords, check for whitespace or EOF at end.
        case 'b':
            // break
            if(std::strncmp("break", data, 5) == 0 && checkEnd(data[5])) {
                base->type = Type::CONTROL;
                base->subType = SubType::BREAK;
                base->value.iof = IOF::UNDEFINED;
                base->size = 5;
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        case 'c':
            // case
            // char
            if(std::strncmp("case", data, 4) == 0 && checkEnd(data[4])) {
                base->type = Type::CONTROL;
                base->subType = SubType::CASE;
                base->value.iof = IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else if(std::strncmp("char", data, 4) == 0) {
                base->type = Type::DATATYPE;
                base->subType = SubType::CHAR;
                base->value.iof = IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'd':
            // default
            if(std::strncmp("default", data, 7) == 0 && checkEnd(data[7])) {
                base->type = Type::CONTROL;
                base->subType = SubType::DEFAULT;
                base->value.iof = IOF::UNDEFINED;
                base->size = 7;
                return data+7;
            } else {
                return parseWord(base, data); 
            }
        case 'e':
            // else
            if(std::strncmp("else", data, 4) == 0 && checkEnd(data[4])) {
                base->type = Type::CONTROL;
                base->subType = SubType::ELSE;
                base->value.iof = IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'f':
            // float
            if(std::strncmp("float", data, 5) == 0 && checkEnd(data[5])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::FLOAT;
                base->value.iof = IOF::PTRLVL;
                base->size = 5+(base->value.holder.ptrLvl = scanPtrLvl(data+5));
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        case 'g':
            // goto
            if(std::strncmp("goto", data, 4) == 0 && checkEnd(data[4])) {
                base->type = Type::CONTROL;
                base->subType = SubType::GOTO;
                base->value.iof = IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'i':
            // if
            // int
            if(std::strncmp("if", data, 2) == 0 && checkEnd(data[2])) {
                base->type = Type::CONTROL;
                base->subType = SubType::IF;
                base->value.iof = IOF::UNDEFINED;
                base->size = 2;
                return data+2;
            } else if(std::strncmp("int", data, 3) == 0 && checkEnd(data[3])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::INT;
                base->value.iof = IOF::PTRLVL;
                base->size = 3+(base->value.holder.ptrLvl = scanPtrLvl(data+3));
                return data+3;
            } else {
                return parseWord(base, data); 
            }
        case 'l':
            // long
            if(std::strncmp("long", data, 4) == 0 && checkEnd(data[4])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::LONG;
                base->value.iof = IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'r':
            // return
            if(std::strncmp("return", data, 6) == 0 && checkEnd(data[6])) {
                base->type = Type::CONTROL;
                base->subType = SubType::RETURN;
                base->value.iof = IOF::UNDEFINED;
                base->size = 6;
                return data+6;
            } else {
                return parseWord(base, data); 
            }
        case 's':
            // switch
            if(std::strncmp("switch", data, 6) == 0 && checkEnd(data[6])) {
                base->type = Type::CONTROL;
                base->subType = SubType::SWITCH;
                base->value.iof = IOF::UNDEFINED;
                base->size = 6;
                return data+6;
            } else {
                return parseWord(base, data); 
            }
        case 'v':
            // void
            if(std::strncmp("void", data, 4) == 0 && checkEnd(data[4])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::VOID;
                base->value.iof = IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'w':
            // while
            if(std::strncmp("while", data, 5) == 0 && checkEnd(data[5])) {
                base->type = Type::CONTROL;
                base->subType = SubType::WHILE;
                base->value.iof = IOF::UNDEFINED;
                base->size = 5;
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        default:
            return parseWord(base, data);
    }
}