#include <string>
#include <vector>
#include <iostream>

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
	virtual llvm::Value* Codegen() = 0;
};

class StatementsAST : public ExprAST {
	public:
	StatementsAST() { this->statements = new std::vector<ExprAST*>(); };
	llvm::BasicBlock* Codegen();
	std::vector<ExprAST*> *statements;
};

class VariableExprAST : public ExprAST {
	std::string Name;

	public:
	VariableExprAST(const std::string &name) : Name(name) {};
	llvm::Value* Codegen();
};

class CallExprAST : public ExprAST {
	std::string Name;
	std::vector<ExprAST*> *Args;

	public:
	CallExprAST(const std::string &name, std::vector<ExprAST*> *args) : Name(name), Args(args) {};
	llvm::Value* Codegen();
};

class FunctionAST {
	std::string Name;
	std::vector<std::string> *Args;
	ExprAST *Body;

	public:
	FunctionAST(const std::string &name, std::vector<std::string> *args, ExprAST *body) : Name(name), Args(args), Body(body) {};
	llvm::Function* Codegen();
	void execute();
};

class IntegerExprAST : public ExprAST {
	int Val;

	public:
	IntegerExprAST(int val) : Val(val) {};
	llvm::Value* Codegen();
};

class BinaryExprAST : public ExprAST {
	int Op;
	ExprAST *LHS, *RHS;

	public:
	BinaryExprAST(int op, ExprAST *lhs, ExprAST *rhs) : Op(op), LHS(lhs), RHS(rhs) {};
	llvm::Value* Codegen();
};

class IfExprAST : public ExprAST {
	ExprAST *Cond, *Then, *Else;

	public:
	IfExprAST(ExprAST *cond, ExprAST *then, ExprAST *_else) : Cond(cond), Then(then), Else(_else) {};
	llvm::Value* Codegen();
};

class UnimplementedAST : public ExprAST {
	public:
	llvm::Value* Codegen();
};
