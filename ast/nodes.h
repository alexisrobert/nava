/*
 * Nava -- LLVM based programming language with automatic generation of JNI interfaces
 * Copyright (C) 2010-2011 by Alexis ROBERT <alexis.robert@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string>
#include <vector>
#include <iostream>

#include "../memory/memory.h"

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>

extern int yyget_lineno();
static llvm::Value *ErrorV(const char *Str) {
	std::cerr << "line " << yyget_lineno() << ": " << Str << std::endl;
	return 0;
}

class ExprAST {
	public:
	virtual llvm::Value* Codegen(VariableTree *memctx) = 0;
};

class StatementsAST : public ExprAST {
	public:
	StatementsAST() { this->statements = new std::vector<ExprAST*>(); };
	llvm::BasicBlock* Codegen(VariableTree *memctx);
	std::vector<ExprAST*> *statements;
};

class VariableExprAST : public ExprAST {
	std::string Name;

	public:
	VariableExprAST(const std::string &name) : Name(name) {};
	std::string& getName();
	llvm::Value* Codegen(VariableTree *memctx);
};

class VariableDefAST : public ExprAST {
	VariableExprAST *LHS;
	ExprAST *RHS;
	const llvm::Type *Type;

	public:
	VariableDefAST(VariableExprAST *lhs, ExprAST *rhs, int type);
	llvm::Value* Codegen(VariableTree *memctx);
	static const llvm::Type* getTypeFromAST(int type);
};

class VariableAssignAST : public ExprAST {
	VariableExprAST *LHS;
	ExprAST *RHS;

	public:
	VariableAssignAST(VariableExprAST *lhs, ExprAST *rhs) : LHS(lhs), RHS(rhs) {};
	llvm::Value* Codegen(VariableTree *memctx);
};

class CallExprAST : public ExprAST {
	std::string Name;
	std::vector<ExprAST*> *Args;

	public:
	CallExprAST(const std::string &name, std::vector<ExprAST*> *args) : Name(name), Args(args) {};
	llvm::Value* Codegen(VariableTree *memctx);
};

class FunctionTypeAST {	
	public:
	FunctionTypeAST(const std::string &name, const llvm::Type *type) : Name(name), Type(type) {};
	std::string Name;
	const llvm::Type *Type;
};

class FunctionAST {
	std::string Name;
	std::vector<FunctionTypeAST*> *Args;
	std::vector<ExprAST*> *Body;
	const llvm::Type *RetType;
	bool native;

	public:
	FunctionAST(const std::string &name,
			std::vector<FunctionTypeAST*> *args,
			std::vector<ExprAST*> *body,
			const llvm::Type *rettype) : Name(name), Args(args), Body(body), RetType(rettype) { this->native = false; };
	
	void setNative(bool native) { this->native = native; };
	llvm::Function* Codegen(VariableTree *memctx);
	void execute();
};

class RootAST {
	public:
	RootAST() { this->children = new std::vector<FunctionAST*>(); };
	std::vector<FunctionAST*> *children;
};

class NumberExprAST : public ExprAST {
	double Val;
	bool DoubleType;

	public:
	NumberExprAST(double val, bool doubleType=false) : Val(val) { this->DoubleType = doubleType; };
	llvm::Value* Codegen(VariableTree *memctx);
};

class BinaryExprAST : public ExprAST {
	int Op;
	ExprAST *LHS, *RHS;

	public:
	BinaryExprAST(int op, ExprAST *lhs, ExprAST *rhs) : Op(op), LHS(lhs), RHS(rhs) {};
	llvm::Value* Codegen(VariableTree *memctx);

	static llvm::Value* add(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	static llvm::Value* sub(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	static llvm::Value* mul(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	static llvm::Value* mod(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	static llvm::Value* div(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
};

class IfExprAST : public ExprAST {
	ExprAST *Cond;
	std::vector<ExprAST*> *Then, *Else;

	public:
	IfExprAST(ExprAST *cond,
			std::vector<ExprAST*> *then,
			std::vector<ExprAST*> *_else) : Cond(cond), Then(then), Else(_else) {};
	llvm::Value* Codegen(VariableTree *memctx);
};

class ForExprAST : public ExprAST {
	ExprAST *Start, *Stop, *Step;
	std::vector<ExprAST*> *Body;

	public:
	ForExprAST(ExprAST *start, ExprAST *stop, ExprAST *step, std::vector<ExprAST*> *body)
				: Start(start), Stop(stop), Step(step), Body(body) {};

	llvm::Value* Codegen(VariableTree *memctx);
};

class UnimplementedAST : public ExprAST {
	public:
	llvm::Value* Codegen(VariableTree *memctx);
};
