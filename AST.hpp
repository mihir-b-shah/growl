
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Parse.h"
#include "Syntax.h"
#include "Error.h"
#include "Vector.hpp"
#include <iostream>
#include <cstdlib>

namespace Parse {
    
    class AST {
    };

    class Expr : public AST {
        friend class ArgIterator;
        public:
            virtual int printRoot(char* buf) const = 0;
            virtual ArgIterator iterator() = 0;
            void print(const int width, std::ostream& out);
    };

	class Control : public AST {
		private:
			char* openBrace;
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
	};

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
			AST* exec;
		public:
			Loop() : Control(nullptr) {
			}
			Loop(char* ob, Expr* _pred, AST* _exec) : Control(ob) {
				pred = _pred;
				exec = _exec;
			}
			~Loop(){
			}
			void setBracket(char* ob){
				
			}
			void setPred(Expr* _pred){
				pred = _pred;
			}	
			void setExec(AST* _exec){
				exec = _exec;
			}
			Expr* getPred(){
				return pred;
			}
			AST* getExec(){
				return exec;
			}
    };

    enum class IntrOps:char {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT, GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, SHIFT};
    
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
            int printRoot(char* buf) const {
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
            Parse::ArgIterator iterator(){
                return ArgIterator(SupportedType::_Lit, this, 0);
            }
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
			void debugPrint(std::ostream& out){
				out << "Name: ";
				out.write(name, len);
				out << " Len: " << static_cast<int>(len);
				out << " VarType: " << varstrs[static_cast<int>(type)];
			   	out << " PtrLvl: " << static_cast<int>(ptrLvl);
				out << '\n';	
			}
    };
	Variable* emptyVar();
	Variable* tombsVar();

	class Sequence : public AST {
		private:
			Utils::SmallVector<AST*, 5> seq;
		public:
			Sequence(){
			}
			~Sequence(){
			}
			void push_back(AST* item){seq.push_back(item);}
			void pop_back(){seq.pop_back();}
			AST* operator[](size_t i){return seq.at(i);}
			size_t size(){return seq.size();}
	};
}

#endif
