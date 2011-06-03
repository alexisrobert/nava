#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <llvm/Support/TypeBuilder.h>

using namespace llvm;

VariableDefAST::VariableDefAST(VariableExprAST *lhs, ExprAST *rhs, int type) {
	this->LHS = lhs;
	this->RHS = rhs;

	this->Type = getTypeFromAST(type);
}

const llvm::Type* VariableDefAST::getTypeFromAST(int type) {
	switch (type) {
	case TDOUBLE:
		return llvm::TypeBuilder<llvm::types::ieee_double, true>().get(llvm::getGlobalContext()); break;
	case TINTEGER:
		return llvm::TypeBuilder<int, false>().get(llvm::getGlobalContext()); break;
	default:
		std::cerr << "Unknown type." << std::endl;
		return llvm::TypeBuilder<void, true>().get(llvm::getGlobalContext()); break;
	}
}

Value *VariableAssignAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Fetch the alloca
	L = memctx->get(LHS->getName());
	if (L == 0) return ErrorV("Variable not found.");

	// Check if R is the same type of L
	if (R->getType()->getPointerTo() != L->getType()) return ErrorV("Different type assignment.");

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
	L = Builder.CreateAlloca(R->getType(), 0, LHS->getName().c_str());

	// TODO: Check the type !!!
	
	memctx->set(LHS->getName(), L);

	// Store the content
	Builder.CreateStore(R, L);

	return R;
}
