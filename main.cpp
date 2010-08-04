#include <iostream>
#include <stdio.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>
#include <llvm/LinkAllPasses.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Target/TargetSelect.h>
#include <llvm/Support/IRBuilder.h>

extern int yyparse();
llvm::Module *TheModule;
llvm::ExecutionEngine *TheExecutionEngine;
llvm::FunctionPassManager *TheFPM;
std::map<std::string, llvm::Value*> NamedValues;
llvm::IRBuilder<> Builder(llvm::getGlobalContext());


int main(int argc, char **argv) {
	TheModule = new llvm::Module("nava", llvm::getGlobalContext());

	/* Initialize the JIT compiler */
	llvm::InitializeNativeTarget();
	TheExecutionEngine = llvm::EngineBuilder(TheModule).create();

	/* Initialize the bytecode optimizer */
	TheFPM = new llvm::FunctionPassManager(TheModule);
	TheFPM->add(new llvm::TargetData(*TheExecutionEngine->getTargetData()));
	TheFPM->add(llvm::createInstructionCombiningPass());
	TheFPM->add(llvm::createReassociatePass());
	TheFPM->add(llvm::createGVNPass());
	TheFPM->add(llvm::createCFGSimplificationPass());
	TheFPM->doInitialization();
	
	yyparse();

	TheModule->dump();
	return 0;
}
