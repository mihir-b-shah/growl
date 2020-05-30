
namespace Global {
    const int InvalidEscapeSequence = 0x10;
    const int InvalidCharacter = 0x11;
    const int InvalidLiteral = 0x12;
    const int InvalidIdentifier = 0x13; // this could also point to an invalid literal
                                        // see the TokenGenerator.cpp where its thrown
}