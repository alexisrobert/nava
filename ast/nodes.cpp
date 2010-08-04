#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <iostream>
#include <stdio.h>

using namespace llvm;

Value *UnimplementedAST::Codegen() {
	std::cerr << "ERROR ! Unimplemented AST node met !" << std::endl;
	return 0;
}

Value *IntegerExprAST::Codegen() {
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), (double)Val);
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
					L = Builder.CreateFCmpULT(L, R, "cmptmp");

					// Convert i1 (boolean) to i32 (signed integer 32bits)
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));
		case TCLEQ:
					L = Builder.CreateFCmpULE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));

		case TCGT:
					L = Builder.CreateFCmpUGT(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));
		case TCGEQ:
					L = Builder.CreateFCmpUGE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));

		case TCEQ:
					L = Builder.CreateFCmpUEQ(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));
		case TCNEQ:
					L = Builder.CreateFCmpUNE(L, R, "cmptmp");
					return Builder.CreateZExt(L, Type::getDoubleTy(getGlobalContext()));
		default: std::cerr << "Invalid binary op met!" << std::endl; return 0;
	}
}

Function *FunctionAST::Codegen() {
	/* Creating function prototype */
	std::vector<const Type*> Integers(Args.size(), Type::getDoubleTy(getGlobalContext()));

	FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()), Integers, false);

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);

	if (Body == 0x00) return 0; // body == 0x00 => this is an extern, we just add it in LLVM's symbol table.

	/* Filling function code */
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	if (Value *RetVal = Body->Codegen()) {
		Builder.CreateRet(Body->Codegen());
	
		/* Verify the function */
		verifyFunction(*F);

		/* Optimize the function */
		TheFPM->run(*F);

		F->dump();

		return F;
	}

	return 0;
}

void FunctionAST::execute() {
	if (Body == 0x00) { this->Codegen()->dump(); return; }

	Function *LF = this->Codegen();

	void *FPtr = TheExecutionEngine->getPointerToFunction(LF);

	double (*FP)() = (double (*)())(intptr_t)FPtr;
	std::cout << "Evaluated to : " << FP() << std::endl;
}
