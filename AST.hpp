
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Parse.h"
#include "Syntax.h"
#include "Error.h"
#include <iostream>
#include <cstdlib>
#include "Vector.hpp"
#include "CodeGen.hpp"

namespace Parse {
   
    class IInstr;    
    class AST {
        public:
            virtual void debugPrint(std::ostream& out) = 0;
            virtual ArgIterator iterator() = 0;
            virtual unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect) = 0;
    };

    class Expr : public AST {
        friend class ArgIterator;
        public:
            virtual int printRoot(char* buf) const = 0;
            void print(const int width, std::ostream& out);
            virtual void debugPrint(std::ostream& out){
                out << "Expr\n";
            }
            virtual unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect) = 0;
    };

    class Sequence : public AST {
        private:
            Utils::SmallVector<AST*,5> seq;
        public:
            Sequence(){
            }
            ~Sequence(){
            }
            /** Returns start iterator */
            AST** begin(){
                return seq.begin();
            }
            /** Returns end iterator */
            AST** end(){
                return seq.end();
            }
            /** Return back element */
            AST* back(){
                return seq.eback();
            }
            AST* at(unsigned int idx){
                return seq[idx];
            }
            void push(AST* item){
                return seq.push_back(item);
            }
            void pop(){
                return seq.pop_back();
            }
            unsigned int size(){
                return seq.size();
            }
            void debugPrint(std::ostream& out){
                out << "sequence\n";
            }
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Seq, this);
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect);
    };

    class Control : public AST {
        private:
            char* openBrace;
            Sequence seq;
        protected:
            Control(char* ob){
                openBrace = ob;
            }
        public:
            Control(){
                openBrace = Lex::program()-1;
            }
            void setBracket(char* ob){
                openBrace = ob;
            }
            char* getBracket(){
                return openBrace;
            }
            unsigned int seqSize(){
                return seq.size();
            }
            void seqAdd(AST* a){
                seq.push(a);
            }
            Sequence* getSeq(){
                return &seq;
            }
            virtual void debugPrint(std::ostream& out){
                out << "Control\n";
            }
            virtual ArgIterator iterator(){
                return ArgIterator(SupportedType::_Ctl, this);
            }
            virtual unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect){
                Global::specifyError("Code gen on Control* not sup.\n");
                throw Global::DeveloperError;
            }
    };
    Control* globScope();

    class FuncDef : public Control {
        private:
            int numArgs;
        public:
            int arity() const {
                Global::specifyError("Unimplemented function call.");
                throw Global::DeveloperError;
            }
    };

    class Branch : public Control {
        friend class ArgIterator;
        private:
            // essentially a link list of branches. last one 
            // guaranteed to have a null predicate pointer.
            Expr* pred = nullptr;
            Branch* next = nullptr;
        public:
            Branch() : Control(nullptr){
            }
            Branch(char* ob, Expr* _pred) : Control(ob){
                pred = _pred;
            }
            ~Branch(){
            }
            Branch* addBranch(){
                next = new Branch();
                return next;
            }
            void setPred(Expr* _pred){
                pred = _pred;
            }    
            Expr* getPred(){
                return pred;
            }
            void debugPrint(std::ostream& out){
                out << "Branch\n";
            }
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Br, this);
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect);
    };

    /*
    later...
        class Switch : Control {
        };

        class Label : Control {
        };
    */

    class Loop : public Control {
        private:
            Expr* pred;
        public:
            Loop() : Control(nullptr) {
            }
            Loop(char* ob, Expr* _pred) : Control(ob) {
                pred = _pred;
            }
            ~Loop(){
            }
            void setBracket(char* ob){
                
            }
            void setPred(Expr* _pred){
                pred = _pred;
            }    
            Expr* getPred(){
                return pred;
            }
            void debugPrint(std::ostream& out){
                out << "Loop\n";
            }
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Lp, this);
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect);
    };

    enum class IntrOps:char {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT, GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, LSHIFT, RSHIFT};
    
    class Op : public Expr {
        friend class ArgIterator;
        private:
            // small size optimization, avoid a heap allocation
            union {
                Expr* arg;
                Expr* twoArgs[2];
                Expr** args;
            } inputs;
            // tag the union
            bool intrinsic;
            union {
                FuncDef* func;
                IntrOps intr;
            } driver;
        public:
            /* code is wrong. constructor should accept for unary/binary
               the small vector optimization */
            Op(FuncDef* def, int argc, Expr** argv);
            Op(FuncDef* def, Expr* e1);
            Op(FuncDef* def, Expr* e1, Expr* e2);
            Op(Lex::SubType op, Expr* e1);
            Op(Lex::SubType op, Expr* e1, Expr* e2);
            ~Op();
            int arity() const;
            int printRoot(char* buf) const;
            ArgIterator iterator();
            void debugPrint(std::ostream& out){
                char buf[5] = {'\0'};
                printRoot(buf);
                out << buf << '\n';
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect);
    };

    class Literal : public Expr {
        friend class ArgIterator;
        private:
            enum:char {
                INT,
                FLOAT
            } type;
            union {
                long long intVal;
                long double fltVal;
            } value;
            static inline int min(int a, int b){
                return a<b?a:b;
            }
        public:
            Literal(){}
            ~Literal(){}
            long long getInt(){ return value.intVal;}
            long double getFlt(){ return value.fltVal;}
            bool isInt(){ return type == INT;}
            bool isFloat(){ return type == FLOAT;}
            void setInt(long long v){
                type = INT;
                value.intVal = v;
            }
            void setFlt(long double v){
                type = FLOAT;
                value.fltVal = v;
            }
            int printRoot(char* buf) const override {
                switch(type){
                    case INT:
                        // print len 3.
                        return min(std::snprintf(buf,4,"%lld",value.intVal),3);
                    case FLOAT:
                        return min(std::snprintf(buf,4,"%lf",static_cast<double>(value.fltVal)),3);
                    default:
                        Global::specifyError("Literal of invalid type.\n");
                        throw Global::DeveloperError;
                }
            }
            Parse::ArgIterator iterator() override {
                return ArgIterator(SupportedType::_Lit, this);
            }
            void debugPrint(std::ostream& out) override {
                char buf[5] = {'\0'};
                printRoot(buf);
                out << buf << '\n';
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect) override;
    };

    enum class VarType:char {INT, LONG, CHAR, FLOAT, BOOL, VOID};
    static const char* varstrs[6] = {"int", "long", "char", "float", "bool", "void"};
    class Variable : public Expr {
        friend class ArgIterator;
        
        typedef unsigned char byte;
        
        const char* name;
        byte len;
        VarType type;
        byte ptrLvl;
        bool _unsigned; // true if so.

        public:
            Variable(){}
            Variable(const char* _name, char _len, Lex::SubType _type, char _ptrLvl);
            ~Variable();
            void set(const char* _name, char _len, Lex::SubType _type, char _ptrLvl);
            char* namePtr(){return const_cast<char*>(name);}
            byte getLen(){return len;}
            int printRoot(char* buf) const override;
            Parse::ArgIterator iterator() override;
            void debugPrint(std::ostream& out) override {
                out << "Name: ";
                out.write(name, len);
                out << " Len: " << static_cast<int>(len);
                out << " VarType: " << varstrs[static_cast<int>(type)];
                   out << " PtrLvl: " << static_cast<int>(ptrLvl);
                out << '\n';    
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect) override;
    };
    Variable* emptyVar();
    Variable* tombsVar();
    
    class Decl : public AST {
        private:
            Variable* var;
            char* index;
        public:
            Decl(Variable* v, char* idx){
                var = v;
                index = idx;
            }
            ~Decl(){
            }
            Variable* getVar(){
                return var;
            }
            int getDist(){
                return index-Lex::program();
            }
            void debugPrint(std::ostream& out){
                var->debugPrint(out);
            }
            Parse::ArgIterator iterator() override {
                return ArgIterator(SupportedType::_Decl, this);
            }
            unsigned int codeGen(Utils::Vector<CodeGen::IInstr>& vect) override;
    };
}

#endif
