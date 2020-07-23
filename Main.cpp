
#include <iostream>
#include <cstdlib>
#include "Lex.h"
#include "Error.h"
#include "AST.hpp"
#include "Allocator.h"
#include "Parse.h"
#include "GroupFinder.hpp"
// #include "SymbolTable.hpp"

// allocator constructed before lexing, freed when compilation ends
Global::Alloc* allocator = nullptr;
Global::Alloc* Global::getAllocator() {
    return allocator;
}

Parse::GroupFinder* groupFinder = nullptr;
Parse::GroupFinder* Parse::gf(){
	return groupFinder;
}

/*
Parse::SymbolTable* symbolTable = nullptr;
Parse::SymbolTable* Parse::st(){
	return symbolTable;
}
*/

static const int FILE_SIZE_MULTIPLIER = 10;
static const int CONSOLE_WIDTH = 100;

int main(int argc, char** argv) {
	std::cout << "HI!\n";
	Global::Alloc alloc(0);
	printf("Line %d\n", __LINE__);
    allocator = &alloc;

    if(argc != 2) {
        std::perror("1 argument needed. Too few/many found.\n");
        return EXIT_FAILURE;
    }
    std::FILE* file = std::fopen(argv[1], "r");
    std::fseek(file, 0L, SEEK_END);
    long size = std::ftell(file);
    
    std::rewind(file);

	printf("Line %d\n", __LINE__);
	// HUGE WARNING MIGHT GET OPTIMIZED AWAY
    char* program = Global::getAllocator()->allocate<char>(size+1);
	printf("Line %d\n", __LINE__);
	
	// ScopedVar::setBase(program);

    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = '\0';
    // lex
    Lex::LexStream tokens(size/sizeof(char));
    try {
		printf("Line %d\n", __LINE__);
        Lex::lex(tokens, size, program);
		printf("Line %d\n", __LINE__);
        tokens.persist("test.txt");
		printf("Line %d\n", __LINE__);
		Parse::GroupFinder _gf(tokens.begin(), tokens.end());
		groupFinder = &_gf;
		printf("Line %d\n", __LINE__);
		// Parse::SymbolTable _st;
		// symbolTable = &_st;
		printf("Line %d\n", __LINE__);
		// Parse::parseLoop(5, tokens.begin()+5, tokens.end());
       
		// Variable* var = Parse::parseDecl(tokens.begin());
		// delete var;
		
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
