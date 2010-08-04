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

					// Convert i1 (boolean) to double
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));
		case TCLEQ:
					L = Builder.CreateFCmpULE(L, R, "cmptmp");
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));

		case TCGT:
					L = Builder.CreateFCmpUGT(L, R, "cmptmp");
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));
		case TCGEQ:
					L = Builder.CreateFCmpUGE(L, R, "cmptmp");
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));

		case TCEQ:
					L = Builder.CreateFCmpUEQ(L, R, "cmptmp");
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));
		case TCNEQ:
					L = Builder.CreateFCmpUNE(L, R, "cmptmp");
					return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));
		default: std::cerr << "Invalid binary op met!" << std::endl; return 0;
	}
}

// TODO : Remove the NamedValue symbol table for a recursive stack based approach
Value *VariableExprAST::Codegen() {
	Value *V = NamedValues[Name];
	return V ? V : ErrorV("Variable name not found in symbol table."); 
}


