#include "nodes.h"
#include "globals.h"

using namespace llvm;

Value *VariableAssignAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Fetch the alloca
	L = memctx->get(LHS->getName());
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
	
	memctx->set(LHS->getName(), L);

	// Store the content
	Builder.CreateStore(R, L);

	return R;
}
