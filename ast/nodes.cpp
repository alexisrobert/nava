#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <iostream>
#include <stdio.h>
#include <llvm/InstrTypes.h>
#include <llvm/Support/TypeBuilder.h>

using namespace llvm;

Value *UnimplementedAST::Codegen (VariableTree *memctx) {
	return ErrorV("Unimplemented AST node met.");
}

Value *NumberExprAST::Codegen (VariableTree *memctx) {
	if (trunc(Val) == Val) { // If the truncated value = the value, it's an int
		return ConstantInt::get(TypeBuilder<int, false>().get(getGlobalContext()),(int)Val);
	} else {
		return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), (double)Val);
	}
}

#define GENERATE_BINARY_OPERATION_MACRO(funcfloat, funcint, tmp) \
	/* If one of the two operands is a double, try to cast the other to double */\
	if (L->getType() == Type::getDoubleTy(getGlobalContext()) || R->getType() == Type::getDoubleTy(getGlobalContext())) {\
		L = Builder.CreateCast(CastInst::getCastOpcode(L, true, Type::getDoubleTy(getGlobalContext()), true),\
				L, Type::getDoubleTy(getGlobalContext()));\
		R = Builder.CreateCast(CastInst::getCastOpcode(R, true, Type::getDoubleTy(getGlobalContext()), true),\
				R, Type::getDoubleTy(getGlobalContext()));\
	}\
	if (L->getType() == TypeBuilder<int,false>().get(llvm::getGlobalContext()) &&\
			L->getType() == TypeBuilder<int,false>().get(llvm::getGlobalContext())) {\
	\
		return funcint(L,R,tmp);\
	\
	} else if (L->getType() == TypeBuilder<llvm::types::ieee_double,false>().get(llvm::getGlobalContext()) &&\
			L->getType() == TypeBuilder<llvm::types::ieee_double,false>().get(llvm::getGlobalContext())) {\
	\
		return funcfloat(L,R,tmp);\
	} else {\
		return ErrorV("Type error while binary operation.");\
	}

// TODO : Implement boolean to remove the double converting crap !
#define GENERATE_BINARY_COMPARAISON_MACRO(funcfloat, funcint, tmp) \
	/* If one of the two operands is a double, try to cast the other to double */\
	if (L->getType() == Type::getDoubleTy(getGlobalContext()) || R->getType() == Type::getDoubleTy(getGlobalContext())) {\
		L = Builder.CreateCast(CastInst::getCastOpcode(L, true, Type::getDoubleTy(getGlobalContext()), true),\
				L, Type::getDoubleTy(getGlobalContext()));\
		R = Builder.CreateCast(CastInst::getCastOpcode(R, true, Type::getDoubleTy(getGlobalContext()), true),\
				R, Type::getDoubleTy(getGlobalContext()));\
	}\
	if (L->getType() == TypeBuilder<int,false>().get(llvm::getGlobalContext()) &&\
			L->getType() == TypeBuilder<int,false>().get(llvm::getGlobalContext())) {\
	\
		L = funcint(L,R,tmp);\
	\
	} else if (L->getType() == TypeBuilder<llvm::types::ieee_double,false>().get(llvm::getGlobalContext()) &&\
			L->getType() == TypeBuilder<llvm::types::ieee_double,false>().get(llvm::getGlobalContext())) {\
	\
		L = funcfloat(L,R,tmp);\
	} else {\
		return ErrorV("Type error while binary comparaison.");\
	} \
	\
	return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()));

Value *BinaryExprAST::add(Value *L, Value *R, VariableTree *memctx) {
	GENERATE_BINARY_OPERATION_MACRO(Builder.CreateFAdd, Builder.CreateAdd, "add")
}

Value *BinaryExprAST::sub(Value *L, Value *R, VariableTree *memctx) {
	GENERATE_BINARY_OPERATION_MACRO(Builder.CreateFSub, Builder.CreateSub, "sub")
}

Value *BinaryExprAST::mul(Value *L, Value *R, VariableTree *memctx) {
	GENERATE_BINARY_OPERATION_MACRO(Builder.CreateFMul, Builder.CreateMul, "mul")
}

Value *BinaryExprAST::mod(Value *L, Value *R, VariableTree *memctx) {
	GENERATE_BINARY_OPERATION_MACRO(Builder.CreateFRem, Builder.CreateSRem, "mod")
}

Value *BinaryExprAST::div(Value *L, Value *R, VariableTree *memctx) {
	GENERATE_BINARY_OPERATION_MACRO(Builder.CreateFDiv, Builder.CreateSDiv, "div")
}

Value *BinaryExprAST::Codegen (VariableTree *memctx) {
	Value *L = LHS->Codegen(memctx);
	Value *R = RHS->Codegen(memctx);
	if (L == 0 || R == 0) return 0;

	switch (Op) {
		case TPLUS: return this->add(L,R,memctx);
		case TMINUS: return this->sub(L,R,memctx);
		case TMULT: return this->mul(L,R,memctx);
		case TMOD: return this->mod(L,R,memctx);
		case TDIV: return this->div(L,R,memctx);

		/* Comparaison operators */
		case TCLT: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpULT, Builder.CreateICmpULT, "cmplt");
		case TCLEQ: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpULE, Builder.CreateICmpULE, "cmpleq");
		case TCGT: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpUGT, Builder.CreateICmpUGT, "cmpgt");
		case TCGEQ: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpUGE, Builder.CreateICmpUGE, "cmpgeq");
		case TCEQ: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpUEQ, Builder.CreateICmpEQ, "cmpeq");
		case TCNEQ: GENERATE_BINARY_COMPARAISON_MACRO(Builder.CreateFCmpUNE, Builder.CreateICmpNE, "cmpneq");
		default: ErrorV("Unknown binary op met!");
	}
}

Value *VariableExprAST::Codegen (VariableTree *memctx) {
	Value *V = memctx->get(Name);

	if (V == 0) return ErrorV((std::string("Variable '")+Name+"' not found in symbol table.").c_str());

	return Builder.CreateLoad(V, Name.c_str());
}

std::string& VariableExprAST::getName() {
	return this->Name;
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

	// Update the memory context to set the variable equal to the PHINode
	AllocaInst *Alloca = VariableTree::CreateEntryBlockAlloca(TheFunction, (*Varname), StartVal->getType());
	newmemctx->set((*Varname), Alloca);

	Value *lastval = Constant::getNullValue(Type::getDoubleTy(getGlobalContext()));

	for (int i = 0; i < Body->size(); i++) {
		if ((lastval = (*Body)[i]->Codegen(newmemctx)) == 0) return 0;
	}

	// Compute step variable
	Value *StepVar = Step->Codegen(newmemctx);
	if (StepVar == 0) return 0;

	// Compute the end condition
	Value *EndCond = Stop->Codegen(newmemctx);
	if (EndCond == 0) return 0;

	EndCond = Builder.CreateFCmpONE(EndCond, ConstantFP::get(getGlobalContext(), APFloat(0.0)), "loopcond");

	// Prepare the "after loop" block, where we'll go if the loop stops
	BasicBlock *LoopEndBB = Builder.GetInsertBlock();
	BasicBlock *AfterBB = BasicBlock::Create(getGlobalContext(), "forend", TheFunction);

	Builder.CreateCondBr(EndCond, LoopBB, AfterBB);

	// Set insert point for new instructions when the loop is finished
	Builder.SetInsertPoint(AfterBB);

	delete newmemctx;

	return lastval;
}
