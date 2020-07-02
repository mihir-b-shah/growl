
#include "Error.h"
#include <cstring>

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
        default:
            std::strncpy(buffer, "Unnamed exception encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
    }
}

// define it once, declared in header file
char Global::errorMsg[Global::ERROR_REFERENCE_SIZE];
void Global::specifyError(const char* spec) {
    std::strncpy(Global::errorMsg, spec, Global::ERROR_REFERENCE_SIZE);
}
