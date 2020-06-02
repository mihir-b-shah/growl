 
#include <cstring>
#include <cctype>
#include "Global.hpp"
#include "Lex.h"

/**
 * Defines a set of keywords. copied from my deformed java code.
 * This is bare bones
 * 
 * Control: if, else, goto, return, while, switch, case, default, break (done!)
 * Data types/modifier: user-def type, int, long, char, float, void, and pointers. (done!)
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
    and a float of arbitrary size. no suffixes either.
    all of this will be handled by a regex enabled preprocessor. */
static inline char* const parseWord(Lex::Token* base, char* const data) {
    if(validLetter(data[0])) {
        // identifier
        int ct = 0;
        // if u actually have a 255 character identifier.... something is very wrong
        // also hopefully this gets vectorized or unrolled....
        while(ct < 255 && std::isalnum(data[ct])) {
            ++ct;
        }
        if(ct == 255) {
            Global::specifyError("Line 40");
            throw Global::InvalidLiteral;
        }
        base->value.iof = Lex::IOF::UNDEFINED;
        base->type = Lex::Type::ID;
        base->subType = Lex::SubType::NAME;
        return data+ct;
    } else if(std::isdigit(data[0])) {
        // int/float
        int ct = 0;
        while(ct < 255 && std::isdigit(data[ct]) && data[ct] != '.') {
            ++ct;
        }
        if(ct == 255 || validLetter(data[ct])) {
            Global::specifyError("Line 51");
            throw Global::InvalidLiteral;
        } else if(data[ct] == '.') {
            // use a float 
            ++ct;
            char* dotPtr = data+ct;
            while(ct < 255 && std::isdigit(data[ct])) {
                ++ct;
            }
            if(ct == 255 || !std::isspace(data[ct])) {
                Global::specifyError("Line 61.");
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
            base->type = Lex::Type::LITERAL;
            base->subType = Lex::SubType::FLT_LITERAL;
            base->value.iof = Lex::IOF::FLOAT_VAL;
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
            base->type = Lex::Type::LITERAL;
            base->subType = Lex::SubType::INT_LITERAL;
            base->value.iof = Lex::IOF::INT_VAL;
            base->value.holder.ival = res;
            return ret;
        }
    } else {
        Global::specifyError(data);
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

static char* const parseCharLiteral(Lex::LitValue::VHolder* holder, char* const ptr) {
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
                Global::specifyError(ptr);
                throw Global::InvalidEscapeSequence;
                break;
        }
        return ptr+4;
    } else {
        Global::specifyError(ptr);
        throw Global::InvalidCharacter;
    }
}

static char* const parse(Lex::Token* base, char* const data) {
    base->size = 1;
    base->pos = data;
    switch(data[0]) {
        case '(':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::OPAREN;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case ')':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::CPAREN;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case ':':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::COLON;
			base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '{':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::OBRACK;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '}':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::CBRACK;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case ',':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::COMMA;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case ';':
            base->type = Lex::Type::GROUP;
            base->subType = Lex::SubType::SEMICOLON;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '+':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::PLUS;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '-':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::MINUS;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '*':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::ASTK;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '/':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::DIV;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '%':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::MOD;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '~':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::NEG;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '.':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::DOT;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '<':
            if(data[1] == '<') {
                base->type = Lex::Type::OPERATOR;
                base->subType = Lex::SubType::SHIFT;
                base->value.iof = Lex::IOF::UNDEFINED;
                return data+2;
            }
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::LESS;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '>':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::GREATER;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '&':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::AMP;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '|':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::OR;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '^':
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::CARET;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '=':
            if(data[1] == '=') {
                base->type = Lex::Type::OPERATOR;
                base->subType = Lex::SubType::EQUAL;
                base->value.iof = Lex::IOF::UNDEFINED;
                return data+2;
            }
            base->type = Lex::Type::OPERATOR;
            base->subType = Lex::SubType::ASSN;
            base->value.iof = Lex::IOF::UNDEFINED;
            return data+1;
        case '\'':
            // char literal
            base->type = Lex::Type::LITERAL;
            base->subType = Lex::SubType::CHAR_LITERAL;
            base->value.iof = Lex::IOF::INT_VAL;
            return parseCharLiteral(&(base->value.holder), data);
        // for all keywords, check for whitespace or EOF at end.
        case 'b':
            // break
            if(std::strncmp("break", data, 5) == 0 && !validLetter(data[5])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::BREAK;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 5;
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        case 'c':
            // case
            // char
            if(std::strncmp("case", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::CASE;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else if(std::strncmp("char", data, 4) == 0) {
                base->type = Lex::Type::DATATYPE;
                base->subType = Lex::SubType::CHAR;
                base->value.iof = Lex::IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'd':
            // default
            if(std::strncmp("default", data, 7) == 0 && !validLetter(data[7])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::DEFAULT;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 7;
                return data+7;
            } else {
                return parseWord(base, data); 
            }
        case 'e':
            // else
            if(std::strncmp("else", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::ELSE;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'f':
            // float
            if(std::strncmp("float", data, 5) == 0 && !validLetter(data[5])) {
                base->type = Lex::Type::DATATYPE;
                base->subType = Lex::SubType::FLOAT;
                base->value.iof = Lex::IOF::PTRLVL;
                base->size = 5+(base->value.holder.ptrLvl = scanPtrLvl(data+5));
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        case 'g':
            // goto
            if(std::strncmp("goto", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::GOTO;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 4;
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'i':
            // if
            // int
            if(std::strncmp("if", data, 2) == 0 && !validLetter(data[2])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::IF;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 2;
                return data+2;
            } else if(std::strncmp("int", data, 3) == 0 && !validLetter(data[3])) {
                base->type = Lex::Type::DATATYPE;
                base->subType = Lex::SubType::INT;
                base->value.iof = Lex::IOF::PTRLVL;
                base->size = 3+(base->value.holder.ptrLvl = scanPtrLvl(data+3));
                return data+3;
            } else {
                return parseWord(base, data); 
            }
        case 'l':
            // long
            if(std::strncmp("long", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Lex::Type::DATATYPE;
                base->subType = Lex::SubType::LONG;
                base->value.iof = Lex::IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'r':
            // return
            if(std::strncmp("return", data, 6) == 0 && !validLetter(data[6])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::RETURN;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 6;
                return data+6;
            } else {
                return parseWord(base, data); 
            }
        case 's':
            // switch
            if(std::strncmp("switch", data, 6) == 0 && !validLetter(data[6])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::SWITCH;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 6;
                return data+6;
            } else {
                return parseWord(base, data); 
            }
        case 'v':
            // void
            if(std::strncmp("void", data, 4) == 0 && !validLetter(data[4])) {
                base->type = Lex::Type::DATATYPE;
                base->subType = Lex::SubType::VOID;
                base->value.iof = Lex::IOF::PTRLVL;
                base->size = 4+(base->value.holder.ptrLvl = scanPtrLvl(data+4));
                return data+4;
            } else {
                return parseWord(base, data); 
            }
        case 'w':
            // while
            if(std::strncmp("while", data, 5) == 0 && !validLetter(data[5])) {
                base->type = Lex::Type::CONTROL;
                base->subType = Lex::SubType::WHILE;
                base->value.iof = Lex::IOF::UNDEFINED;
                base->size = 5;
                return data+5;
            } else {
                return parseWord(base, data); 
            }
        default:
            return parseWord(base, data);
    }
}

void Lex::lex(Lex::LexStream& tokens, char* const program) {
    char* moving = program;
    while(*moving != '\0') {
        Lex::Token* token = tokens.allocate();
        while(isspace(*moving)) {
            ++moving;
        }
        if(!isgraph(*moving)) {
            break;
        }
        moving = parse(token, moving);
    }
}