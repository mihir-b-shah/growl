
#include <cstdio>
#include <cstdlib>
#include "Lex.h"
#include "Error.h"
#include "AST.h"
#include "Allocator.h"
#include "SmallVector.hpp"

// allocator constructed before lexing, freed when compilation ends
Global::Alloc* allocator = nullptr;
Global::Alloc* Global::getAllocator() {
    return allocator;
}

static const int FILE_SIZE_MULTIPLIER = 10;

int main(int argc, char** argv) {
    Global::Alloc alloc(0);
    allocator = &alloc;

    if(argc != 2) {
        std::perror("1 argument needed. Too few/many found.\n");
        return EXIT_FAILURE;
    }
    std::FILE* file = std::fopen(argv[1], "r");
    std::fseek(file, 0L, SEEK_END);
    long size = std::ftell(file);
    
    std::rewind(file);
    char* program = Global::getAllocator()->allocate<char>(size+1);

    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = '\0';
    // lex
    Lex::LexStream tokens(size/sizeof(char));
    try {
        Lex::lex(tokens, program);
        tokens.persist("test.txt");
        Global::getAllocator()->deallocate<char>(program);
        return EXIT_SUCCESS;
    } catch (int exc) {
        char buffer[Global::ERROR_BUFFER_SIZE];
        Global::genError(buffer, exc);
        printf("\n%s\n", buffer);
        printf("%s\n", Global::errorMsg);
        Global::getAllocator()->deallocate<char>(program);
        return EXIT_FAILURE;
    }
}