
#include "Global.h"
#include <cstring>

void Global::genError(char buffer[Global::ERROR_BUFFER_SIZE], int error) {
    switch(error) {
        case Global::InvalidEscapeSequence:
            std::strncpy(buffer, "Lex error: Invalid escape sequence encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
        case Global::InvalidCharacter:
            std::strncpy(buffer, "Lex error: Invalid character encountered.", 
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
        default:
            std::strncpy(buffer, "Unnamed exception encountered.", 
                        Global::ERROR_BUFFER_SIZE);
            break;
    }
}