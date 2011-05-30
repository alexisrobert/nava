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
	llvm::Type *Type;

	public:
	VariableDefAST(VariableExprAST *lhs, ExprAST *rhs, int type);
	llvm::Value* Codegen(VariableTree *memctx);
	static llvm::Type* getTypeFromAST(int type);
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
	FunctionTypeAST(const std::string &name, llvm::Type *type) : Name(name), Type(type) {};
	std::string Name;
	llvm::Type *Type;
};

class FunctionAST {
	std::string Name;
	std::vector<FunctionTypeAST*> *Args;
	std::vector<ExprAST*> *Body;
	bool native;

	public:
	FunctionAST(const std::string &name,
			std::vector<FunctionTypeAST*> *args,
			std::vector<ExprAST*> *body) : Name(name), Args(args), Body(body) { this->native = false; };
	
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

	public:
	NumberExprAST(double val) : Val(val) {};
	llvm::Value* Codegen(VariableTree *memctx);
};

class BinaryExprAST : public ExprAST {
	int Op;
	ExprAST *LHS, *RHS;

	public:
	BinaryExprAST(int op, ExprAST *lhs, ExprAST *rhs) : Op(op), LHS(lhs), RHS(rhs) {};
	llvm::Value* Codegen(VariableTree *memctx);

	llvm::Value* add(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	llvm::Value* sub(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	llvm::Value* mul(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	llvm::Value* mod(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
	llvm::Value* div(llvm::Value *L, llvm::Value *R, VariableTree *memctx);
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
	std::string *Varname;

	public:
	ForExprAST(std::string *varname, ExprAST *start, ExprAST *stop, ExprAST *step, std::vector<ExprAST*> *body)
				: Start(start), Stop(stop), Step(step), Body(body), Varname(varname) {};

	llvm::Value* Codegen(VariableTree *memctx);
};

class UnimplementedAST : public ExprAST {
	public:
	llvm::Value* Codegen(VariableTree *memctx);
};
