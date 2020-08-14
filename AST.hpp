
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

    class AST {
        unsigned id;
        protected:
            AST(){
                id = getASTCtr();
                incrASTCtr();
            }
        public:
            unsigned getHash(){return id;}
            virtual ArgIterator iterator() = 0;
            virtual unsigned int codeGen(CodeGen::IRProg& prog) = 0;
            virtual void fixTypes() = 0;
    };

    enum class ExprId:char {_OP, _LIT, _VAR, _CAST};
    class Expr : public AST {
        friend class ArgIterator;
        protected:
            VarType _in;
        public:
            virtual VarType castType() = 0;
            virtual ExprId exprID() = 0;
            virtual int printRoot(char* buf) const = 0;
            void print(const int width, std::ostream& out);
            unsigned int codeGen(CodeGen::IRProg& prog);
            void fixTypes() override;
            VarType getType(){return _in;}
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

            /* Implemented in ASTParser */
            AST* getSequentialBase();
            void setBack(Sequence* _back, unsigned _idx){
                back = _back;
                idx = _idx;
            }
            // never will be called.
            ArgIterator iterator() override { 
                return ArgIterator(SupportedType::_Ctl, this); 
            }
            unsigned int codeGen(CodeGen::IRProg& prog){return 0;}
            virtual void fixTypes(){}
    };

    class Sequence : public AST {
        friend class Control;
        private:
            Utils::SmallVector<AST*,6> seq;
        protected:
            void setBackTrace(Sequence* _back, unsigned _idx){
                getControlNode()->setBack(_back, _idx);
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
            ControlNode* getControlNode(){
                return static_cast<ControlNode*>(seq[seq.size()-1]);
            }
            AST* getSequential(){
                return getControlNode()->getSequentialBase();
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
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Seq, this);
            }
            unsigned int codeGen(CodeGen::IRProg& prog);
            void fixTypes() override;
    };

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
                return static_cast<ControlNode*>(seq.getControlNode());
            }
            void seqAdd(AST* a){
                seq.push(a);
            }
            Sequence* getSeq(){
                return &seq;
            }
            virtual ArgIterator iterator(){
                return ArgIterator(SupportedType::_Ctl, this);
            }
            virtual unsigned int codeGen(CodeGen::IRProg& prog){
                Global::specifyError("Code gen on Control* not sup.\n", __FILE__, __LINE__);
                throw Global::DeveloperError;
            }
            virtual void fixTypes() override;
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

    // Else statement is guaranteeed to exist.
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
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Br, this);
            }
            unsigned int codeGen(CodeGen::IRProg& prog);
            void fixTypes() override;
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
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Lp, this);
            }
            unsigned int codeGen(CodeGen::IRProg& prog);
            void fixTypes() override;
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
            ExprId exprID() {return ExprId::_OP;}
            Expr* getUnaryArg(){return inputs.arg;}
            Expr* getBinaryArg1(){return inputs.twoArgs[0];}
            Expr* getBinaryArg2(){return inputs.twoArgs[1];}
            void setUnaryArg(Expr* v){inputs.arg = v;}
            void setBinaryArg1(Expr* v){inputs.twoArgs[0] = v;}
            void setBinaryArg2(Expr* v){inputs.twoArgs[1] = v;}
            IntrOps getIntrinsicOp(){return driver.intr;}
            bool isIntrinsic(){return intrinsic;}
            VarType castType();
    };

    /* This is an operator inaccessible to the user */
    class Cast : public Expr {
        private:
            VarType out;
            Expr* in;         
            static inline int min(int a, int b){
                return a<b?a:b;
            }
        public:
            Cast(Expr*, VarType);
            ~Cast();
            VarType getCastType(){return out;}
            Expr* getExpr(){return in;}
            VarType castType() override {
                Global::specifyError("Should never be called.\n",
                                __FILE__, __LINE__);
                throw Global::DeveloperError;
            }
            ArgIterator iterator(){
                return ArgIterator(SupportedType::_Cast, this);
            }
            ExprId exprID() override {return ExprId::_CAST;}
            int printRoot(char* buf) const override {
                switch(out){
                    case VarType::CHAR:
                        return min(std::snprintf(buf, 4, "CHR"),3);
                    case VarType::BOOL:
                        return min(std::snprintf(buf, 4, "BLN"),3);
                    case VarType::FLOAT:
                        return min(std::snprintf(buf, 4, "FLT"),3);
                    case VarType::INT:
                        return min(std::snprintf(buf, 4, "INT"),3);
                    case VarType::LONG:
                        return min(std::snprintf(buf, 4, "LNG"),3);
                    default:
                        Global::specifyError("Cast not possible.\n", 
                            __FILE__, __LINE__);
                        throw Global::InvalidCast;
                }
                return 0;
            }
    };

    class Literal : public Expr {
        friend class ArgIterator;
        private:
            enum:char {
                INT,
                FLOAT,
            } type;
            // literals can never be negative.
            union {
                unsigned long long intVal;
                double fltVal;
            } value;
            static inline int min(int a, int b){
                return a<b?a:b;
            }
        public:
            Literal(){}
            ~Literal(){}
            unsigned long long getInt(){ return value.intVal;}
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
                        return min(std::snprintf(buf,4,"%lf",value.fltVal),3);
                    default:
                        Global::specifyError("Literal of invalid type.\n", __FILE__, __LINE__);
                        throw Global::DeveloperError;
                }
            }
            Parse::ArgIterator iterator() override {
                return ArgIterator(SupportedType::_Lit, this);
            }
            ExprId exprID() override {return ExprId::_LIT;}
            VarType castType();
            void convType(bool);
    };

    static const char* varstrs[7] = {"int", "long", "char", "float", "bool", "void", "other"};
    class Variable : public Expr {
        friend class ArgIterator;
        
        typedef unsigned char byte;
        
        const char* name;
        byte len;
        VarType type;
        byte ptrLvl;

        public:
            Variable(){}
            Variable(const char* _name, char _len, Lex::SubType _type, char _ptrLvl);
            ~Variable();
            void set(const char* _name, char _len, Lex::SubType _type, char _ptrLvl);
            char* namePtr(){return const_cast<char*>(name);}
            byte getLen(){return len;}
            ExprId exprID() override {return ExprId::_VAR;}
            int printRoot(char* buf) const override;
            Parse::ArgIterator iterator() override;
            VarType castType();
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
            VarType castType(){
                return var->castType();
            }
            int getDist(){
                return index-Lex::program();
            }
            Parse::ArgIterator iterator() override {
                return ArgIterator(SupportedType::_Decl, this);
            }
            unsigned int codeGen(CodeGen::IRProg& prog) override;
            ExprId exprID(){return ExprId::_VAR;}
            void fixTypes() override {}
    };
}

#endif
