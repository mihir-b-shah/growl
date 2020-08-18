

#include <cstring>
#include <iostream>
#include <cstdlib>

#include "Lex.h"
#include "Error.h"
#include "AST.hpp"
#include "Allocator.h"
#include "Parse.h"
#include "GroupFinder.hpp"
#include "SymbolTable.hpp"
#include "CodeGen.hpp"

/* 
 * LETS FIX ERROR.H, USE THE __FILE__ AND __LINE__ MACROS
 * TO GET BETTER ERROR MESSAGES
 */


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

unsigned ssaLblCtr = 1;
CodeGen::SSA CodeGen::nextSSA(){
    return CodeGen::SSA(ssaLblCtr++);
}

CodeGen::Label CodeGen::nextLabel(){
    return CodeGen::Label(ssaLblCtr++);
}

unsigned ASTCtr = 0;
unsigned int Parse::getASTCtr(){
    return ASTCtr;
}
void Parse::incrASTCtr(){
    ++ASTCtr;
}

// guaranteed to be exclusive since a
// variable cant be a control str.
union SSALbl {
    CodeGen::SSA ssa;
    CodeGen::Label lbl;

    SSALbl(){
        ssa = CodeGen::SSA::nullValue();
    }
};

struct VarSSA {
    int extr;
    unsigned ssa;

    VarSSA(){extr = 0;}

    VarSSA(int e, unsigned s){
        extr = e;
        ssa = s;
    }
};

template<>
struct Utils::SetTraits<VarSSA> {
    static VarSSA emptyVal(){return VarSSA(0,0);}
    static VarSSA tombstoneVal(){return VarSSA(-1,0);}
    static size_t hash(VarSSA x){return static_cast<size_t>(x.extr);}
    static bool equal(VarSSA v1, VarSSA v2){return v1.extr == v2.extr;}
};

Utils::SmallSet<VarSSA,50> varPtrs;
void CodeGen::insertVarPtr(unsigned int Var_Extr, SSA ssa){
    varPtrs.insert(VarSSA(Var_Extr, ssa.extractLbl()));
}

CodeGen::SSA CodeGen::getVarPtr(unsigned int Var_Extr){
    VarSSA* res = varPtrs.find(VarSSA(Var_Extr, 0));
    return res == nullptr ? CodeGen::SSA::nullValue()
            : CodeGen::SSA(res->ssa);
}

Utils::SmallVector<SSALbl,50> labels;
CodeGen::Label CodeGen::getFromAST(unsigned AST_Extract){
    return labels[AST_Extract].lbl;
}
void CodeGen::insertASTLbl(unsigned AST_Extr, CodeGen::Label lbl){
    labels.ref(AST_Extr).lbl = lbl;
}

void CodeGen::insertVarSSA(unsigned int Var_Extr, CodeGen::SSA ssa){
    labels.ref(Var_Extr).ssa = ssa;
}
CodeGen::SSA CodeGen::getFromVar(unsigned int Var_Extract){
    return labels[Var_Extract].ssa;
}

static const int FILE_SIZE_MULTIPLIER = 10;
static const int CONSOLE_WIDTH = 100;

// i just want this to be heap allocated for some reason.
static void singletonVars(){
    // char is a dummy here
    _tombsVar = new Parse::Variable("TOMBS_3022", 10, Lex::SubType::CHAR, 0);
    _emptyVar = new Parse::Variable("EMPTY_2930", 10, Lex::SubType::CHAR, 0);    
}

CodeGen::IRProg irProg;
CodeGen::IRProg& getIRProg(){
    return irProg;
}

void test(CodeGen::IInstr& node){
    char buf[50] = {'\0'};
    node.output(buf);
    std::cout << buf << '\n';
}

int main(int argc, char** argv) {
    Global::Alloc alloc(0);
    allocator = &alloc;

    /*
    CodeGen::SSA s1 = CodeGen::nextSSA();
    CodeGen::SSA s2 = CodeGen::nextSSA();
    CodeGen::SSA s3 = CodeGen::nextSSA(); 

    CodeGen::IInstr ins;
    ins = CodeGen::IInstr(Parse::IntrOps::ADD, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::MINUS, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::MULT, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::DIV, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::MOD, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::LSHIFT, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::RSHIFT, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::AND, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::OR, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::XOR, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::GREATER, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::LESS, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::EQUAL, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::ASSN, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::FLIP, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::IntrOps::NEG, Parse::VarType::INT, s1, s2, s3); 
    test(ins);
    ins = CodeGen::IInstr(Parse::VarType::INT, s1); 
    test(ins);
    ins = CodeGen::IInstr(Parse::VarType::INT, s1); 
    test(ins);
    ins = CodeGen::IInstr(CodeGen::MemAction::LOAD, Parse::VarType::INT, s1, s3); 
    test(ins);
    ins = CodeGen::IInstr(CodeGen::MemAction::STORE, Parse::VarType::INT, s1, s3); 
    test(ins);
    */
    
    if(argc != 2) {
        std::perror("1 argument needed. Too few/many found.\n");
        return EXIT_FAILURE;
    }
    char buf[81] = {'\0'};
    std::strncpy(buf, "testfiles/", 11);
    std::strncat(buf, argv[1], 69);
    std::FILE* file = std::fopen(buf, "r");
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
        
        // Parsing
        Parse::parseAST(0, tokens.begin(), tokens.end(), 
                        Parse::globScope());

        // Static type analysis (add casting)
        Parse::globScope()->fixTypes();

        CodeGen::IRProg irProg;
        CodeGen::genIR(irProg);
        
        constexpr unsigned int OUTFILE_LEN = 80;
        char outFileBuf[1+OUTFILE_LEN] = {'\0'};
        std::strncpy(outFileBuf, "llvmfiles/", 11);
        std::strncat(outFileBuf, argv[1], std::strrchr(argv[1], '.')-argv[1]);
        std::strncat(outFileBuf, ".ll", 4);
        std::ofstream irFile(outFileBuf);
        irProg.write(irFile);
        irFile.flush();
        irFile.close();

        return EXIT_SUCCESS;
    } catch (int exc) {
        char buffer[Global::ERROR_BUFFER_SIZE];
        Global::genError(buffer, exc);
        std::printf("\n%s\n", buffer);
        std::printf("%s\n", Global::errorMsg);
        std::printf("%s\n", Global::location);
        Global::getAllocator()->deallocate<char>(program);
        return EXIT_FAILURE;
    }
}
