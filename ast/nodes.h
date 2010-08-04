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
	llvm::BasicBlock* Codegen();
	std::vector<ExprAST*> *statements;
};

class FunctionAST {
	std::string Name;
	std::vector<std::string> Args;
	ExprAST *Body;

	public:
	FunctionAST(const std::string &name, const std::vector<std::string> &args, ExprAST *body) : Name(name), Args(args), Body(body) {};
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

class UnimplementedAST : public ExprAST {
	public:
		llvm::Value* Codegen();
};
