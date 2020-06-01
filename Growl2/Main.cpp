
#include <cstdio>
#include <cstdlib>
#include "Lex.h"
#include "Global.h"

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
    char* program = static_cast<char*>(std::malloc(size+sizeof(char)));
    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = '\0';

    // lex
    Lex::LexStream tokens(size/sizeof(char));
    try {
        Lex::lex(tokens, program);
        tokens.persist("test.txt");
        return EXIT_SUCCESS;
    } catch (int exc) {
        char buffer[Global::ERROR_BUFFER_SIZE];
        Global::genError(buffer, exc);
        printf("%s\n", buffer);
        printf("%s\n", Global::errorMsg);
        return EXIT_FAILURE;
    } 
}