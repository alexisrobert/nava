#include "nodes.h"
#include "globals.h"

using namespace llvm;

Value *IntegerExprAST::Codegen() {
	return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), Val);
}

llvm::Function *FunctionAST::Codegen() {
	/* Creating function prototype */
	std::vector<const Type*> Integers(Args.size(), Type::getInt32Ty(getGlobalContext()));

	FunctionType *FT = FunctionType::get(Type::getInt32Ty(getGlobalContext()), Integers, false);

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);

	/* Filling function code */
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);
	
	//verifyFunction(*F);

	return F;
}
