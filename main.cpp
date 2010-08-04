#include <iostream>
#include <stdio.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetSelect.h>

extern int yyparse();
llvm::Module *TheModule;
llvm::ExecutionEngine *TheExecutionEngine;
llvm::FunctionPassManager *TheFPM;

int main(int argc, char **argv) {
	TheModule = new llvm::Module("nava", llvm::getGlobalContext());

	/* Initialize the JIT compiler */
	llvm::InitializeNativeTarget();
	TheExecutionEngine = llvm::EngineBuilder(TheModule).create();

	/* Initialize the optimizer */
	TheFPM = new llvm::FunctionPassManager(TheModule);
	TheFPM->add(new llvm::TargetData(*TheExecutionEngine->getTargetData()));
	// Add passes here
	TheFPM->doInitialization();
	
	yyparse();

	TheModule->dump();
	return 0;
}
