
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
  
    unsigned int getASTCtr();
    void incrASTCtr();

    class IInstr;    
    class AST {
        unsigned id;
        protected:
            AST(){
                id = getASTCtr();
                incrASTCtr();
            }
        public:
            unsigned getHash(){return id;}
            virtual void debugPrint(std::ostream& out) = 0;
            virtual ArgIterator iterator() = 0;
            virtual unsigned int codeGen(CodeGen::IRProg prog) = 0;
    };

    class Expr : public AST {
        friend class ArgIterator;
        public:
            virtual int printRoot(char* buf) const = 0;
            void print(const int width, std::ostream& out);
            virtual void debugPrint(std::ostream& out){
                out << "Expr\n";
            }
            virtual unsigned int codeGen(CodeGen::IRProg prog) = 0;
    };

    class Sequence;
    /*
     * How to manage labels.
     *
     * Keep a static integer for the AST class.
     * Each time an AST gets allocated assign it the current value
     * and increment.
     *
     * Now create a Map<AST_Int, Label>. Look before creating label.
     */
    class ControlNode : public AST {
        private:
            Sequence* back; 
            unsigned idx;      
        public:
            ControlNode(){
            }
            ~ControlNode(){
            }
            Sequence* getBack(){return back;}
            unsigned getIdx(){return idx;}
            AST* getSequential();
            void setBack(Sequence* _back, unsigned _idx){
                back = _back;
                idx = _idx;
            }
            void debugPrint(std::ostream& out){
                out << "ControlNode\n";
            }
            // never will be called.
            ArgIterator iterator(){ 
                return ArgIterator(SupportedType::_Ctl, this); 
            }
            // never will be called.
            unsigned int codeGen(CodeGen::IRProg prog){
                return 0;
            }
    };

    class Sequence : public AST {
        friend class Control;
        private:
            Utils::SmallVector<AST*,6> seq;
        protected:
            void setBackTrace(Sequence* _back, unsigned _idx){
                static_cast<ControlNode*>(back())
                        ->setBack(_back, _idx);
            }
        public:
            Sequence(){
                // a single null AST at the "back"
                // helps in transferring control.
                seq.push_back(new ControlNode());
            }
            ~Sequence(){
            }
            /** Returns start iterator */
            AST** begin(){
                return seq.begin();
            }
            /** Returns end iterator */
            AST** end(){
                return seq.end() - 1;
            }
            /** Return back element. Dont inclue ControlNode */
            AST* back(){
                return seq.at(seq.size()-2);
            }
            // prob should never be called.
            AST* at(unsigned int idx){
                return seq[idx];
            }
            void push(AST* item){
                seq.push_back(item);
                seq.swap(seq.size()-2, seq.size()-1); 
            }
            void pop(){
                seq.pop_back();
            }
            unsigned int size(){
                return seq.size()-1;
            }
            void debugPrint(std::ostream& out){
                out << "sequence\n";
            }
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Seq, this);
            }
            unsigned int codeGen(CodeGen::IRProg prog);
    };

    // Associated method with control node, bc forward decl.
    AST* ControlNode::getSequential(){
        return getBack()->at(getIdx()+1);
    }

    class Control : public AST {
        private:
            char* openBrace;
            Sequence seq;
            void setBackTrace(Sequence* _back, unsigned _idx){
                seq.setBackTrace(_back, _idx);
            }
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
            void setBackTrace(Control* parent){
                seq.setBackTrace(parent->getSeq(),
                                parent->getSeq()->size()-1);
            }
            void setBackTrace(ControlNode* same){
                seq.setBackTrace(same->getBack(), same->getIdx());
            }
            ControlNode* getBackTrace(){
                return static_cast<ControlNode*>(seq.back());
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
            virtual unsigned int codeGen(CodeGen::IRProg prog){
                Global::specifyError("Code gen on Control* not sup.\n", __FILE__, __LINE__);
                throw Global::DeveloperError;
            }
    };
    Control* globScope();

    class FuncDef : public Control {
        private:
            int numArgs;
        public:
            int arity() const {
                Global::specifyError("Unimplemented function call.", __FILE__, __LINE__);
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
                next->setBackTrace(getBackTrace());
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
            unsigned int codeGen(CodeGen::IRProg prog);
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
            unsigned int codeGen(CodeGen::IRProg prog);
    };

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
            unsigned int codeGen(CodeGen::IRProg prog);
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
                double fltVal;
            } value;
            static inline int min(int a, int b){
                return a<b?a:b;
            }
        public:
            Literal(){}
            ~Literal(){}
            long long getInt(){ return value.intVal;}
            double getFlt(){ return value.fltVal;}
            bool isInt(){ return type == INT;}
            bool isFloat(){ return type == FLOAT;}
            void setInt(long long v){
                type = INT;
                value.intVal = v;
            }
            void setFlt(double v){
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
                        Global::specifyError("Literal of invalid type.\n", __FILE__, __LINE__);
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
            unsigned int codeGen(CodeGen::IRProg prog) override;
    };

    static const char* varstrs[7] = {"int", "long", "char", "float", "bool", "void", "other"};
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
            unsigned int codeGen(CodeGen::IRProg prog) override;
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
            void debugPrint(std::ostream& out) override {
                var->debugPrint(out);
            }
            Parse::ArgIterator iterator() override {
                return ArgIterator(SupportedType::_Decl, this);
            }
            unsigned int codeGen(CodeGen::IRProg prog) override;
    };
}

#endif
