#include <string>
#include <vector>

#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>

class ExprAST {
	public:
		virtual llvm::Value* Codegen() {};
};

class StatementsAST : public ExprAST {
	public:
	StatementsAST() { this->statements = new std::vector<ExprAST*>(); };
	std::vector<ExprAST*> *statements;
};

class FunctionAST {
	std::string Name;
	std::vector<std::string> Args;
	StatementsAST *Body;

	public:
	FunctionAST(const std::string &name, const std::vector<std::string> &args, StatementsAST *body) : Name(name), Args(args), Body(body) {};
	llvm::Function* Codegen();
};

class IntegerExprAST : public ExprAST {
	int Val;

	public:
	IntegerExprAST(int val) : Val(val) {};
	llvm::Value* Codegen();
};

class VariableExprAST : public ExprAST {
	std::string Name;

	public:
	VariableExprAST(const std::string &name) : Name(name) {};
};

class BinaryExprAST : public ExprAST {
	int Op;
	ExprAST *LHS, *RHS;

	public:
	BinaryExprAST(int op, ExprAST *lhs, ExprAST *rhs) : Op(op), LHS(lhs), RHS(rhs) {};
};

class ReturnStmtAST : public ExprAST {
	ExprAST* Retval;

	public:
	ReturnStmtAST(ExprAST* retval) : Retval(retval) {};
};

class UnimplementedAST : public ExprAST {
};
