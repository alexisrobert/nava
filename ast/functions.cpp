#include "nodes.h"
#include "globals.h"

using namespace llvm;

Function *FunctionAST::Codegen(VariableTree *memctx) {
	VariableTree *newmemctx = new VariableTree(memctx); // New function's memory context

	/* Creating function prototype */
	std::vector<const Type*> FuncArgs(this->Args->size(), Type::getDoubleTy(getGlobalContext()));

	FunctionType *FT = FunctionType::get(Type::getDoubleTy(getGlobalContext()), FuncArgs, false);

	Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule);

	if (Body == 0x00) return 0; // body == 0x00 => this is an extern, we just add it in LLVM's symbol table.

	/* Naming arguments (btw, we don't need that for externs) */
	unsigned Idx = 0;
	for (Function::arg_iterator AI = F->arg_begin(); Idx != Args->size(); ++AI, ++Idx) {
		AI->setName((*this->Args)[Idx]);

		newmemctx->set((*Args)[Idx], AI); // Update symbol table
	}

	/* Filling function code */
	BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", F);
	Builder.SetInsertPoint(BB);

	/* Inserting all statements */
	Value *last_value = 0;
	
	for (int i = 0; i < Body->size(); i++) {
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
