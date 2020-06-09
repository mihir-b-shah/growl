
#ifndef ERROR_H
#define ERROR_H

namespace Global {
    const int DeveloperError = 0xF;
    const int InvalidEscapeSequence = 0x10;
    const int InvalidCharacter = 0x11;
    const int InvalidLiteral = 0x12;
    const int InvalidIdentifier = 0x13; // this could also point to an invalid literal
                                        // see the TokenGenerator.cpp where its thrown
    const int InvalidOperator = 0x14; 
	const int InvalidFunctionCall = 0x15;
	const int InvalidOperatorInvocation = 0x16;

    const int ERROR_BUFFER_SIZE = 255;
    const int ERROR_REFERENCE_SIZE = 31;

    static const int GROWTH_FACTOR = 2;

    extern char errorMsg[ERROR_REFERENCE_SIZE];

    void genError(char buffer[ERROR_BUFFER_SIZE], int error);
    void specifyError(const char* spec);
}

#endif