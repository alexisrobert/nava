#include "nodes.h"
#include "globals.h"

using namespace llvm;

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

		return F;
	}

	// TODO : Error occured during bytecode generation, remove the function from symbols.

	return 0;
}

void FunctionAST::execute() {
	Function *LF = this->Codegen();
	if (Body == 0 || LF != 0) LF->dump();

	if (LF == 0) {
		return;
	}

	if (this->Args.size() != 0) {
		std::cout << "Function with multiple arguments, not executing." << std::endl;
		return;
	}

	void *FPtr = TheExecutionEngine->getPointerToFunction(LF);

	double (*FP)() = (double (*)())(intptr_t)FPtr;
	std::cout << "Evaluated to : " << FP() << std::endl;
}
