#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <iostream>
#include <stdio.h>

using namespace llvm;

Value *UnimplementedAST::Codegen (VariableTree *memctx) {
	return ErrorV("Unimplemented AST node met.");
}

Value *IntegerExprAST::Codegen (VariableTree *memctx) {
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), (double)Val);
}

Value *BinaryExprAST::Codegen (VariableTree *memctx) {
	Value *L = LHS->Codegen(memctx);
	Value *R = RHS->Codegen(memctx);
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
Value *VariableExprAST::Codegen (VariableTree *memctx) {
	Value *V = memctx->get(Name);

	return V ? V : ErrorV((std::string("Variable '")+Name+"' not found in symbol table.").c_str()); 
}

Value *CallExprAST::Codegen (VariableTree *memctx) {
	Function *CalleeF = TheModule->getFunction(Name);
	if (CalleeF == 0)
		return ErrorV((std::string("Unknown function : ")+Name).c_str());

	if (CalleeF->arg_size() != Args->size())
		return ErrorV((std::string("Wrong argument numbers for '")+Name+"'").c_str());

	std::vector<Value*> ArgsV;

	// Expanding argument values
	for (unsigned int i = 0, e = Args->size(); i != e; ++i) {
		ArgsV.push_back((*Args)[i]->Codegen(memctx));
		if (ArgsV.back() == 0) return 0;
	}

	return Builder.CreateCall(CalleeF, ArgsV.begin(), ArgsV.end(), "calltmp");
}

Value *IfExprAST::Codegen (VariableTree *memctx) {
	// Create the comparaison part
	Value *CondV = Cond->Codegen(memctx);
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
	VariableTree *ThenMemctx = new VariableTree(memctx);

	for (int i = 0; i < Then->size(); i++) {
		ThenV = (*Then)[i]->Codegen(ThenMemctx);

		if (ThenV == 0) return 0;
	}

	Builder.CreateBr(MergeBB);
	ThenBB = Builder.GetInsertBlock(); // Update ThenBB pointer for PHI

	delete ThenMemctx;

	// Insert into the "else" part
	TheFunction->getBasicBlockList().push_back(ElseBB);
	Builder.SetInsertPoint(ElseBB);

	Value *ElseV = 0;
	VariableTree *ElseMemctx = new VariableTree(memctx);
	for (int i = 0; i < Else->size(); i++) {
		ElseV = (*Else)[i]->Codegen(ElseMemctx);

		if (ElseV == 0) return 0;
	}

	Builder.CreateBr(MergeBB);
	ElseBB = Builder.GetInsertBlock(); // Update ElseBB ptr for PHI

	delete ElseMemctx;

	// Create the merge block and PHI stuff
	TheFunction->getBasicBlockList().push_back(MergeBB);
	Builder.SetInsertPoint(MergeBB);

	PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), "iftmp");
	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);

	return PN;
}

Value *ForExprAST::Codegen(VariableTree *memctx) {
	VariableTree *newmemctx = new VariableTree(memctx);

	// Get start value
	Value *StartVal = Start->Codegen(memctx);
	if (StartVal == 0) return 0;

	// Create blocks
	Function *TheFunction = Builder.GetInsertBlock()->getParent(); // Returns the current function
	BasicBlock *PreheaderBB = Builder.GetInsertBlock(); // Returns the current insert block

	BasicBlock *LoopBB = BasicBlock::Create(getGlobalContext(), "for", TheFunction);

	Builder.CreateBr(LoopBB);
	Builder.SetInsertPoint(LoopBB); // We now insert code in the loop

	// Add a PHI node for the initial value (if we come from start => StartVal, else use the updated value)
	PHINode *Variable = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), Varname->c_str());

	Variable->addIncoming(StartVal, PreheaderBB);

	// Update the memory context to set the variable equal to the PHINode
	newmemctx->set((*Varname), Variable);
	for (int i = 0; i < Body->size(); i++) {
		if ((*Body)[i]->Codegen(newmemctx) == 0) return 0;
	}

	// Compute step variable
	Value *StepVar = Step->Codegen(newmemctx);
	if (StepVar == 0) return 0;

	Value *NextVar = Builder.CreateFAdd(Variable, StepVar, "nextvar");

	// Compute the end condition
	Value *EndCond = Stop->Codegen(newmemctx);
	if (EndCond == 0) return 0;

	EndCond = Builder.CreateFCmpONE(EndCond, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "loopcond");

	// Prepare the "after loop" block, where we'll go if the loop stops
	BasicBlock *LoopEndBB = Builder.GetInsertBlock();
	BasicBlock *AfterBB = BasicBlock::Create(getGlobalContext(), "forend", TheFunction);

	Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

	// Update the PHI node (<=> update the variable after a loop)
	Variable->addIncoming(NextVar, LoopEndBB);

	// Set insert point for new instructions when the loop is finished
	Builder.SetInsertPoint(AfterBB);

	return Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));
}
