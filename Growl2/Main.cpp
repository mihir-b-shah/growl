
#include <cstdio>
#include <cstdlib>
#include "Lex.h"

int main(int argc, char** argv) {
    if(argc != 2) {
        std::perror("1 argument needed. Too few/many found.\n");
        return EXIT_FAILURE;
    }
    std::FILE* file = std::fopen(argv[1], "r");
    std::fseek(file, 0L, SEEK_END);
    long size = std::ftell(file);
    std::rewind(file);
    // only to avoid doing in terms of char new.
    char* program = static_cast<char*>(std::malloc(size+1));
    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = 0;

    // lex
    Lex::LexStream lexStream(size/sizeof(char));
    Lex::lex(lexStream, program);

    return EXIT_SUCCESS;
}