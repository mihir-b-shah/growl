
#include <iostream>
#include <cstdlib>

#include "Lex.h"
#include "Error.h"
#include "AST.hpp"
#include "Allocator.h"
#include "Parse.h"
#include "GroupFinder.hpp"
#include "SymbolTable.hpp"


// allocator constructed before lexing, freed when compilation ends
Global::Alloc* allocator = nullptr;
Global::Alloc* Global::getAllocator() {
    return allocator;
}

Parse::GroupFinder* groupFinder = nullptr;
Parse::GroupFinder* Parse::gf(){
	return groupFinder;
}

Parse::SymbolTable* symbolTable = nullptr;
Parse::SymbolTable* Parse::st(){
	return symbolTable;
}

char* _program = nullptr;
char* Lex::program(){
	return _program;
}

Parse::Variable* _emptyVar = nullptr;
Parse::Variable* Parse::emptyVar(){
	return _emptyVar;
}

Parse::Variable* _tombsVar = nullptr;
Parse::Variable* Parse::tombsVar(){
	return _tombsVar;
}

Control _globScope;
Control* Parse::globScope(){
	return &_globScope;
}

static const int FILE_SIZE_MULTIPLIER = 10;
static const int CONSOLE_WIDTH = 100;

// i just want this to be heap allocated for some reason.
static void singletonVars(){
	// char is a dummy here
	_tombsVar = new Parse::Variable("TOMBS_3022", 10, Lex::SubType::CHAR, 0);
	_emptyVar = new Parse::Variable("EMPTY_2930", 10, Lex::SubType::CHAR, 0);	
}

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

	// HUGE WARNING MIGHT GET OPTIMIZED AWAY
    char* program = Global::getAllocator()->allocate<char>(size+1);
	_program = program;
	
    std::fread(program, 1, size, file);
    std::fflush(file);
    std::fclose(file);
    program[size] = '\0';
    // lex
    Lex::LexStream tokens(size/sizeof(char));
	try {
        Lex::lex(tokens, size, program);
        tokens.persist("test.txt");
		Parse::GroupFinder _gf(tokens.begin(), tokens.end());
		groupFinder = &_gf;
		singletonVars();
		Parse::SymbolTable _st;
		symbolTable = &_st;
		
		Parse::parseAST(0, tokens.begin(), tokens.end(), 
						Parse::globScope());

		Global::getAllocator()->deallocate<char>(program);
//		return EXIT_SUCCESS;
    } catch (int exc) {
        char buffer[Global::ERROR_BUFFER_SIZE];
        Global::genError(buffer, exc);
        printf("\n%s\n", buffer);
        printf("%s\n", Global::errorMsg);
        Global::getAllocator()->deallocate<char>(program);
        return EXIT_FAILURE;
    }
}
