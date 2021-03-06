
#include "Error.h"
#include <cstring>
#include <cstdio>

void Global::genError(char buffer[Global::ERROR_BUFFER_SIZE], int error) {
    switch(error) {
        case Global::MemoryRequestError:
            std::strncpy(buffer, "Too much memory requested.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidEscapeSequence:
            std::strncpy(buffer, "Lex error: Invalid escape sequence encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidCharacter:
            std::strncpy(buffer, "Lex error: Invalid ASCII character encountered. Note- this could be anything.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidLiteral:
            std::strncpy(buffer, "Lex error: Invalid literal encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidIdentifier:
            std::strncpy(buffer, "Lex error: Invalid identifier/literal encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidOperator:
            std::strncpy(buffer, "Lex error: Invalid operator encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::DeveloperError:
            std::strncpy(buffer, "Developer error, see details.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidFunctionCall:
            std::strncpy(buffer, "Parse error, error in func. call invocation.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidOperatorInvocation:
            std::strncpy(buffer, "Parse error, operator called with an incorrect number of args.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidExpression:
            std::strncpy(buffer, "Invalid expression encountered in parsing", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidLoop:
            std::strncpy(buffer, "Invalid loop.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidDeclaration:
            std::strncpy(buffer, "Invalid declaration.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidBranch:
            std::strncpy(buffer, "Invalid branch.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidInstrInvocation:
            std::strncpy(buffer, "Invalid use of instruction.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::NotSupportedError:
            std::strncpy(buffer, "Not supported yet.", Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidCast:
            std::strncpy(buffer, "Cast not supported.", Global::ERROR_BUFFER_SIZE);
            break;
        default:
            std::strncpy(buffer, "Unnamed exception encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
    }
}

// define it once, declared in header file
char Global::errorMsg[Global::ERROR_REFERENCE_SIZE];
char Global::location[Global::FILE_REFERENCE_SIZE];

// https://stackoverflow.com/questions/8487986/file-macro-shows-full-path

static inline const char* extractFile(const char* file){
    const char* ptr1 = std::strrchr(file, '/');
    const char* ptr2 = std::strrchr(file, '\\');
    
    if(__builtin_expect(ptr1 == nullptr && ptr2 == nullptr,false)){
        return file;
    } else {
        return ptr1 == nullptr ? ptr2+1 : ptr1+1;
    }
}

void Global::specifyError(const char* spec, const char* file, unsigned int line) {
    std::snprintf(Global::location, Global::FILE_REFERENCE_SIZE, 
                    "File: %s, Line:%u\n", extractFile(file), line);
    std::strncpy(Global::errorMsg, spec, Global::ERROR_REFERENCE_SIZE);
}
