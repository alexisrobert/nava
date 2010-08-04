#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <iostream>

using namespace llvm;

Value *IntegerExprAST::Codegen() {
	return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), Val);
}

Value *BinaryExprAST::Codegen() {
	Value *L = LHS->Codegen();
	Value *R = RHS->Codegen();
	if (L == 0 || R == 0) return 0;

	switch (Op) {
		case TPLUS: return Builder.CreateAdd(L,R,"addtmp");
		case TMINUS: return Builder.CreateSub(L,R,"subtmp");
		case TMULT: return Builder.CreateMul(L,R,"multmp");

		/* Comparaison operators */
		case TCLT:
					L = Builder.CreateICmpSLT(L, R, "cmptmp");

					// Convert i1 (boolean) to i32 (signed integer 32bits)
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));
		case TCLEQ:
					L = Builder.CreateICmpSLE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));

		case TCGT:
					L = Builder.CreateICmpSGT(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));
		case TCGEQ:
					L = Builder.CreateICmpSGE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));

		case TCEQ:
					L = Builder.CreateICmpEQ(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));
		case TCNEQ:
					L = Builder.CreateICmpNE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getInt32Ty(getGlobalContext()));
		default: std::cerr << "Invalid binary op met!" << std::endl; return 0;
	}
}

Function *FunctionAST::Codegen() {
	/* Creating function prototype */
	std::vector<const Type*> Integers(Args.size(), Type::getInt32Ty(getGlobalContext()));

	FunctionType *FT = FunctionType::get(Type::getInt32Ty(getGlobalContext()), Integers, false);

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);

	/* Filling function code */
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	Builder.CreateRet(Body->Codegen());
	
	verifyFunction(*F);

	return F;
}
