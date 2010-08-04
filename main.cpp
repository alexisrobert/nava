#include <iostream>
#include <stdio.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>

extern int yyparse();
llvm::Module *TheModule;

int main(int argc, char **argv) {
	llvm::LLVMContext &Context = llvm::getGlobalContext();
	TheModule = new llvm::Module("nava", Context);
	
	yyparse();

	TheModule->dump();
	return 0;
}
