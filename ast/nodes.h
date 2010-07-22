#include <string>
#include <vector>

class ExprAST {
};

class IntegerExprAST : public ExprAST {
	int Val;

	public:
	IntegerExprAST(int val) : Val(val) {}
};

class VariableExprAST : public ExprAST {
	std::string Name;

	public:
	VariableExprAST(const std::string &name) : Name(name) {};
};

class BinaryExprAST : public ExprAST {
	int Op;
	ExprAST *LHS, *RHS;
	bool Native;

	public:
	BinaryExprAST(int op, ExprAST *lhs, ExprAST *rhs, bool native = true) : Op(op), LHS(lhs), RHS(rhs), Native(native) {};
};

class StatementsAST {
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
};

class UnimplementedAST : public ExprAST {
};
