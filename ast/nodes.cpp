#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <iostream>
#include <stdio.h>

using namespace llvm;

Value *UnimplementedAST::Codegen() {
	return ErrorV("Unimplemented AST node met.");
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
		default: ErrorV("Unknown binary op met!");
	}
}

// TODO : Remove the NamedValue symbol table for a recursive stack based approach
Value *VariableExprAST::Codegen() {
	Value *V = NamedValues[Name];
	return V ? V : ErrorV("Variable name not found in symbol table."); 
}

Value *CallExprAST::Codegen() {
	Function *CalleeF = TheModule->getFunction(Name);
	if (CalleeF == 0)
		return ErrorV("Unknown function");

	if (CalleeF->arg_size() != Args->size())
		return ErrorV("Arguments number mismatch");

	std::vector<Value*> ArgsV;
	for (unsigned int i = 0, e = Args->size(); i != e; ++i) {
		ArgsV.push_back((*Args)[i]->Codegen());
		if (ArgsV.back() == 0) return 0;
	}

	return Builder.CreateCall(CalleeF, ArgsV.begin(), ArgsV.end(), "calltmp");
}
