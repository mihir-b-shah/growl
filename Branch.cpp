
#include "Parse.h"
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"
#include <iostream>

using namespace Parse;

/**
 * Syntax:
 * if (bool predicate){
 *     some AST.
 * }
 *
 */
Lex::Token* Parse::parseBranch(int _offset, Lex::Token* _begin, Branch* _br){
    // this corner case is safe since the ASTParser already validates base case.
    if(_begin->subType == SubType::OBRACK){    
        int matchBrack = gf()->find(_offset, 0);
        parseAST(_offset+1, _begin + 1, _begin + matchBrack, _br);
        return _begin + matchBrack + 1;
    }    

    int idx = 0;
    if(__builtin_expect((_begin+idx)->subType != SubType::IF,false)){
        Global::specifyError("\"If\" token not found at _branch start.\n", __FILE__, __LINE__);
        throw Global::InvalidBranch;
    }
    ++idx;
    if(__builtin_expect((_begin+idx)->subType != SubType::OPAREN,false)){
        Global::specifyError("Opening paren not found after if.\n", __FILE__, __LINE__);
        throw Global::InvalidBranch;
    }
    int matchParen = gf()->find(_offset,idx);
    Expr* predicate = parseExpr(_begin+idx+1, _begin+matchParen);
    _br->setPred(predicate);
    _br->setBracket(_offset+const_cast<char*>(_begin->pos));

    idx = matchParen+1;
    if(__builtin_expect((_begin+idx)->subType != SubType::OBRACK, false)){
        Global::specifyError("Opening _bracket not found after if.\n", __FILE__, __LINE__);
        throw Global::InvalidBranch;
    }

    int matchBrack = gf()->find(_offset, idx);
    parseAST(_offset + matchParen + 2, _begin + matchParen + 2, 
                    _begin + matchBrack, _br);
    
    // hopefully gets tail call optimized
    if((_begin+matchBrack+1)->subType == SubType::ELSE){
        return parseBranch(_offset+matchBrack+2, _begin+matchBrack+2, _br->addBranch());
    } else {
        return _begin+matchBrack+1;    
    }    
}
