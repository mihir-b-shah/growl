
#include <cstdio>
#include <cstdlib>
#include "Lex.h"
#include "Global.hpp"

// allocator constructed before lexing, freed when compilation ends
Global::Alloc* allocator = nullptr;
Global::Alloc* Global::getAllocator() {
    return allocator;
}

static const int FILE_SIZE_MULTIPLIER = 10;

int main(int argc, char** argv) {
    if(argc != 2) {
        std::perror("1 argument needed. Too few/many found.\n");
        return EXIT_FAILURE;
    }
    std::FILE* file = std::fopen(argv[1], "r");
    std::fseek(file, 0L, SEEK_END);
    long size = std::ftell(file);
    std::rewind(file);
   
    Global::Alloc alloc = Global::Alloc(FILE_SIZE_MULTIPLIER*size);
    allocator = &alloc;
    
    allocator->printDebug("MAIN");
    // problem is here, my allocator is screwing up
    char* program = alloc.allocate<char>(size/sizeof(char)+1);
    std::printf("Amount requested: %d\n", size/sizeof(char)+1);
    allocator->printDebug("MAIN");

    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = '\0';

    std::printf("at line %d in main: %s\n", __LINE__, program);
    // lex
    Lex::LexStream tokens(size/sizeof(char));
    try {
        std::printf("at line %d in main: %s\n", __LINE__, program);
        Lex::lex(tokens, program);
        tokens.persist("test.txt");
        return EXIT_SUCCESS;
    } catch (int exc) {
        char buffer[Global::ERROR_BUFFER_SIZE];
        Global::genError(buffer, exc);
        printf("\n%s\n", buffer);
        printf("%s\n", Global::errorMsg);
        return EXIT_FAILURE;
    } 
}