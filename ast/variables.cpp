#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

using namespace llvm;

VariableDefAST::VariableDefAST(VariableExprAST *lhs, ExprAST *rhs, int type) {
	this->LHS = lhs;
	this->RHS = rhs;

	this->Type = getTypeFromAST(type);
}

VariableType VariableDefAST::getTypeFromAST(int type) {
	switch (type) {
	case TDOUBLE:
		return DOUBLE; break;
	default:
		std::cerr << "Unknown type." << std::endl;
		return UNDEFINITE; break;
	}
}

Value *VariableAssignAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Fetch the alloca
	VariableType type = memctx->getType(LHS->getName());

	L = memctx->get(LHS->getName(), type);
	if (L == 0) return ErrorV("Variable not found.");

	// Store the content
	Builder.CreateStore(R, L);
	return R;
}

Value *VariableDefAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Create the alloca
	L = Builder.CreateAlloca(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 0, LHS->getName().c_str());
	
	memctx->set(LHS->getName(), this->Type, L);

	// Store the content
	Builder.CreateStore(R, L);

	return R;
}
