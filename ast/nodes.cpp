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
		case TPLUS: return Builder.CreateFAdd(L,R,"addtmp");
		case TMINUS: return Builder.CreateFSub(L,R,"subtmp");
		case TMULT: return Builder.CreateFMul(L,R,"multmp");
		case TMOD: return Builder.CreateFRem(L,R,"modtmp");

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
	return V ? V : ErrorV((std::string("Variable '")+Name+"' not found in symbol table.").c_str()); 
}

Value *CallExprAST::Codegen() {
	Function *CalleeF = TheModule->getFunction(Name);
	if (CalleeF == 0)
		return ErrorV((std::string("Unknown function : ")+Name).c_str());

	if (CalleeF->arg_size() != Args->size())
		return ErrorV((std::string("Wrong argument numbers for '")+Name+"'").c_str());

	std::vector<Value*> ArgsV;
	for (unsigned int i = 0, e = Args->size(); i != e; ++i) {
		ArgsV.push_back((*Args)[i]->Codegen());
		if (ArgsV.back() == 0) return 0;
	}

	return Builder.CreateCall(CalleeF, ArgsV.begin(), ArgsV.end(), "calltmp");
}

Value *IfExprAST::Codegen() {
	// Create the comparaison part
	Value *CondV = Cond->Codegen();
	if (CondV == 0) return 0;

	CondV = Builder.CreateFCmpONE(CondV, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "ifcond");

	// Create the condition switch
	Function *TheFunction = Builder.GetInsertBlock()->getParent();

	BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then", TheFunction);
	BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
	BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

	Builder.CreateCondBr(CondV, ThenBB, ElseBB);
	Builder.SetInsertPoint(ThenBB);

	// Insert into the "then" part
	Value *ThenV = 0;	
	for (int i = 0; i < Then->size(); i++) {
		ThenV = (*Then)[i]->Codegen();

		if (ThenV == 0) return 0;
	}

	Builder.CreateBr(MergeBB);
	ThenBB = Builder.GetInsertBlock(); // Update ThenBB pointer for PHI

	// Insert into the "else" part
	TheFunction->getBasicBlockList().push_back(ElseBB);
	Builder.SetInsertPoint(ElseBB);

	Value *ElseV = 0;
	for (int i = 0; i < Else->size(); i++) {
		ElseV = (*Else)[i]->Codegen();

		if (ElseV == 0) return 0;
	}

	Builder.CreateBr(MergeBB);
	ElseBB = Builder.GetInsertBlock(); // Update ElseBB ptr for PHI

	// Create the merge block and PHI stuff
	TheFunction->getBasicBlockList().push_back(MergeBB);
	Builder.SetInsertPoint(MergeBB);

	PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), "iftmp");
	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);

	return PN;
}
