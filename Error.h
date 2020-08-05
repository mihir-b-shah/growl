
#ifndef ERROR_H
#define ERROR_H

namespace Global {
    
    const int MemoryRequestError = 0xE;
    const int DeveloperError = 0xF;
    
    const int InvalidEscapeSequence = 0x10;
    const int InvalidCharacter = 0x11;
    const int InvalidLiteral = 0x12;
    // this could also point to an invalid literal
    // see the TokenGenerator.cpp where its thrown
    const int InvalidIdentifier = 0x13; 
    
    const int InvalidOperator = 0x14; 
    const int InvalidFunctionCall = 0x15;
    const int InvalidOperatorInvocation = 0x16;
    const int InvalidExpression = 0x17;
    const int InvalidLoop = 0x18;
    const int InvalidDeclaration = 0x19;
    const int InvalidBranch = 0x1A;
    const int InvalidInstrInvocation = 0x1B;

    const int ERROR_BUFFER_SIZE = 255;
    const int ERROR_REFERENCE_SIZE = 31;
    const int FILE_REFERENCE_SIZE = 31;

    static const int GROWTH_FACTOR = 2;

    extern char location[FILE_REFERENCE_SIZE];
    extern char errorMsg[ERROR_REFERENCE_SIZE];

    void genError(char buffer[ERROR_BUFFER_SIZE], int error);
    void specifyError(const char* spec, const char* file, unsigned int line);
}

#endif
