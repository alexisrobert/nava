#include "nodes.h"
#include "globals.h"

using namespace llvm;

Function *FunctionAST::Codegen(VariableTree *memctx) {
	VariableTree *newmemctx = new VariableTree(memctx); // New function's memory context

	/* Creating function prototype */
	std::vector<const Type*> *FuncArgs = new std::vector<const Type*>();
	for (unsigned i = 0; i < this->Args->size(); i++) {
		FuncArgs->push_back((*this->Args)[i]->Type);
	}

	/* Add JNI variables for environment and parent if native. */
	if (this->native == true) {
		FuncArgs->insert(FuncArgs->begin(), 2, PointerType::get(OpaqueType::get(getGlobalContext()),0));
	}

	FunctionType *FT = FunctionType::get(RetType, *FuncArgs, false);

	delete FuncArgs;

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);

	if (Body == 0x00) return 0; // body == 0x00 => this is an extern, we just add it in LLVM's symbol table.

	/* Creating the basic block which will hold the function */
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	/* Naming arguments (btw, we don't need that for externs) */
	unsigned Idx = 0;
	Function::arg_iterator AI = F->arg_begin();

	/* Sets variable names for environment and parent if native. */
	if (this->native == true) {
		AI->setName("env");
		AI++;
		AI->setName("parent");
		AI++;
	}

	for (; Idx != Args->size(); ++AI, ++Idx) {
		AI->setName((*this->Args)[Idx]->Name);

		// Create the alloca for this variable
		AllocaInst *Alloca = VariableTree::CreateEntryBlockAlloca(F, (*this->Args)[Idx]->Name, (*this->Args)[Idx]->Type);

		// Store the initial value in the alloca
		Builder.CreateStore(AI, Alloca);

		// And the update the symbol table
		newmemctx->set((*this->Args)[Idx]->Name, Alloca);
	}

	/* Inserting all statements */
	Value *last_value = 0;
	
	for (unsigned int i = 0; i < Body->size(); i++) {
		last_value = (*Body)[i]->Codegen(newmemctx); // Codegen and insert

		if (last_value == 0) break;
	}

	delete newmemctx;

	if (last_value != 0) {
		Builder.CreateRet(last_value);
	
		/* Verify the function */
		verifyFunction(*F);

		/* Optimize the function */
		TheFPM->run(*F);

		return F;
	}

	// TODO : Error occured during bytecode generation, remove the function from symbols.

	return 0;
}

void FunctionAST::execute() {
	Function *LF = this->Codegen(new VariableTree());

	if (LF == 0) {
		return;
	}

	//LF->dump();

	// We only execute functions with no arguments
	if (this->Args->size() != 0) {
		return;
	}

	void *FPtr = TheExecutionEngine->getPointerToFunction(LF);

	double (*FP)() = (double (*)())(intptr_t)FPtr;
	std::cout << Name << "() returns " << FP() << std::endl;
}
