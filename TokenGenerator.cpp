 
#include <cstring>
#include <cctype>
#include "Error.h"
#include "Lex.h"
#include <cstdio>

using namespace Lex;

/**
 * Defines a set of keywords. copied from my deformed java code.
 * This is bare bones
 * 
 * Control: if, else, goto, return, while, switch, case, default, break (done!)
 * Data types/modifier: user-def type, int, long, char, double, void, and pointers. (done!)
 * Group: (, ), :, {, }, ',', ';' (done!)
 * Literals: number, char, floating-pt. (done!)
 * Operators: +, -, /, %, ~, ., *, <, >, &, |, ^, =, ==, << (done!)
 * Identifier: letter1/underscore, then any number too. no $. (done!)
 * 
 * Hardcoded token generator.
 */

static inline bool validLetter(const char letter) {
    return std::isalpha(letter) || letter == '_';
}

/*  parse an identifier or int/flt litera
    right now... only a decimal int literal of arbitrary size
    and a double of arbitrary size. no suffixes either.
    all of this will be handled by a regex enabled preprocessor. */
static inline char* const parseWord(Token* base, char* const data) {
    if(validLetter(data[0])) {
        // identifier
        int ct = 0;
        // if u actually have a 255 character identifier.... something is very wrong
        // also hopefully this gets vectorized or unrolled....
        while(ct < 255 && std::isalnum(data[ct])) {
            ++ct;
        }
        if(ct == 255) {
            Global::specifyError("String too long.", __FILE__, __LINE__);
            throw Global::InvalidLiteral;
        }
        base->size = ct;
        base->value.iof = IOF::UNDEFINED;
        base->type = Type::ID;
        base->subType = SubType::NAME;
        return data+ct;
    } else if(std::isdigit(data[0])) {
        // int/double
        int ct = 0;
        while(ct < 255 && std::isdigit(data[ct]) && data[ct] != '.') {
            ++ct;
        }
        if(ct == 255 || validLetter(data[ct])) {
            Global::specifyError("Invalid numeric literal.", __FILE__, __LINE__);
            throw Global::InvalidLiteral;
        } else if(data[ct] == '.') {
            // use a double
            // might have bugs... 
            char* dotPtr = data+ct;
            ++ct;
            while(ct < 255 && std::isdigit(data[ct])) {
                ++ct;
            }
            if(ct == 255 || validLetter(data[ct])) {
                Global::specifyError("Invalid floating point literal.", __FILE__, __LINE__);
                throw Global::InvalidLiteral;
            }
            double res = 0;
            char* readPtr = data;
            while(readPtr != dotPtr){
                res *= 10;
                res += *readPtr-'0';
                ++readPtr;
            }
            readPtr = dotPtr+1;
            double power = 0.1;
            while(readPtr != data+ct) {
                res += power*(*readPtr-'0');
                ++readPtr;
                power *= 0.1;
            }
            base->size = ct;
            base->type = Type::LITERAL;
            base->subType = SubType::FLT_LITERAL;
            base->value.iof = IOF::FLOAT_VAL;
            base->value.holder.fval = res;
            return readPtr;
        } else {
            // use an int, num ranges from [data, data+ct).
            char* readPtr = data;
            long long res = 0;
            while(readPtr != data+ct) {
                res *= 10;
                res += *readPtr-'0';
                ++readPtr;
            }
            base->size = ct;
            base->type = Type::LITERAL;
            base->subType = SubType::INT_LITERAL;
            base->value.iof = IOF::INT_VAL;
            base->value.holder.ival = res;
            return readPtr;
        }
    } else {
        Global::specifyError(data, __FILE__, __LINE__);
        throw Global::InvalidIdentifier;
    }
}

static inline int scanPtrLvl(char* const ptr) {
    int ct = 0;
    while(*(ptr+ct) == '*') {
        ++ct;
    }
    return ct;
}

static char* const parseCharLiteral(LitValue::VHolder* holder, char* const ptr) {
    if(*(ptr+2) == '\'') {
        // a proper character
        holder->ival = *(ptr+1);
        return ptr+3;
    } else if(*(ptr+1) == '\\' && *(ptr+3) == '\'') {
        // same as java, escape sequences.
        switch(*(ptr+2)) {
            case 't':
                holder->ival = '\t';
                break;
            case 'b':
                holder->ival = '\b';
                break;
            case 'n':
                holder->ival = '\n';
                break;
            case 'r':
                holder->ival = '\r';
                break;
            case 'f':
                holder->ival = '\t';
                break;
            case '0':
                holder->ival = '\0';
                break;
            case '\'':
                holder->ival = '\'';
                break;
            case '\"':
                holder->ival = '\"';
                break;
            case '\\':
                holder->ival = '\\';
                break;
            default:
                Global::specifyError(ptr, __FILE__, __LINE__);
                throw Global::InvalidEscapeSequence;
                break;
        }
        return ptr+4;
    } else {
        Global::specifyError(ptr, __FILE__, __LINE__);
        throw Global::InvalidCharacter;
    }
}

static char* const parse(Token* base, char* const data) {
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
            if(data[1] == '<') {
                base->size = 2;
                base->type = Type::OPERATOR;
                base->subType = SubType::LSHIFT;
                base->value.iof = IOF::UNDEFINED;
                return data+2;
            }
            base->type = Type::OPERATOR;
            base->subType = SubType::LESS;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '>':
            if(data[1] == '>') {
                base->size = 2;
                base->type = Type::OPERATOR;
                base->subType = SubType::RSHIFT;
                base->value.iof = IOF::UNDEFINED;
                return data+2;
            }
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
            if(data[1] == '=') {
                base->size = 2;
                base->type = Type::OPERATOR;
                base->subType = SubType::EQUAL;
                base->value.iof = IOF::UNDEFINED;
                return data+2;
            }
            base->type = Type::OPERATOR;
            base->subType = SubType::ASSN;
            base->value.iof = IOF::UNDEFINED;
            return data+1;
        case '\'':
            // char literal
            base->type = Type::LITERAL;
            base->subType = SubType::CHAR_LITERAL;
            base->value.iof = IOF::INT_VAL;
            return parseCharLiteral(&(base->value.holder), data);
        // for all keywords, check for whitespace or EOF at end.
        case 'b':
            // break
            if(std::strncmp("break", data, 5) == 0 && !validLetter(data[5])) {
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
            if(std::strncmp("case", data, 4) == 0 && !validLetter(data[4])) {
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
                return data+4+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'd':
            // default
            if(std::strncmp("default", data, 7) == 0 && !validLetter(data[7])) {
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
            if(std::strncmp("else", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Type::CONTROL;
                base->subType = SubType::ELSE;
                base->value.iof = IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'f':
            // double
            if(std::strncmp("float", data, 5) == 0 && !validLetter(data[5])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::FLOAT;
                base->value.iof = IOF::PTRLVL;
                base->size = 5+(base->value.holder.ptrLvl = scanPtrLvl(data+5));
                return data+5+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'g':
            // goto
            if(std::strncmp("goto", data, 4) == 0 && !validLetter(data[4])) {
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
            if(std::strncmp("if", data, 2) == 0 && !validLetter(data[2])) {
                base->type = Type::CONTROL;
                base->subType = SubType::IF;
                base->value.iof = IOF::UNDEFINED;
                base->size = 2;
                return data+2;
            } else if(std::strncmp("int", data, 3) == 0 && !validLetter(data[3])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::INT;
                base->value.iof = IOF::PTRLVL;
                base->size = 3+(base->value.holder.ptrLvl = scanPtrLvl(data+3));
                return data+3+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'l':
            // long
            if(std::strncmp("long", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::LONG;
                base->value.iof = IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'r':
            // return
            if(std::strncmp("return", data, 6) == 0 && !validLetter(data[6])) {
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
            if(std::strncmp("switch", data, 6) == 0 && !validLetter(data[6])) {
                base->type = Type::CONTROL;
                base->subType = SubType::SWITCH;
                base->value.iof = IOF::UNDEFINED;
                base->size = 6;
                return data+6;
            } else {
                return parseWord(base, data); 
            }
        case 'u':
            // unsigned
            if(std::strncmp("unsigned", data, 8) == 0 && !validLetter(data[8])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::UNSIGNED;
                base->value.iof = IOF::PTRLVL;
                base->size = 8+(base->value.holder.ptrLvl = scanPtrLvl(data+8));
                return data+8+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'v':
            // void
            if(std::strncmp("void", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Type::DATATYPE;
                base->subType = SubType::VOID;
                base->value.iof = IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4+base->value.holder.ptrLvl;
            } else {
                return parseWord(base, data); 
            }
        case 'w':
            // while
            if(std::strncmp("while", data, 5) == 0 && !validLetter(data[5])) {
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

void Lex::lex(LexStream& tokens, int size, char* const program) {
    char* moving = program;
    while(isspace(*moving)) {
        ++moving;
    }
    while(*moving != '\0') {
        Token* token = tokens.allocate();
        moving = parse(token, moving);
        while(isspace(*moving)) {
            ++moving;
        }
    }
}
