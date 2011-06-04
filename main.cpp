/*
 * Nava -- LLVM based programming language with automatic generation of JNI interfaces
 * Copyright (C) 2010-2011 by Alexis ROBERT <alexis.robert@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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

#include "ast/nodes.h"

extern int yyparse();
llvm::Module *TheModule;
llvm::ExecutionEngine *TheExecutionEngine;
llvm::FunctionPassManager *TheFPM;
std::map<std::string, llvm::Value*> NamedValues;
llvm::IRBuilder<> Builder(llvm::getGlobalContext());

RootAST *rootnode = new RootAST();

int main(int argc, char **argv) {
	TheModule = new llvm::Module("nava", llvm::getGlobalContext());

	/* Initialize the JIT compiler */
	llvm::InitializeNativeTarget();
	TheExecutionEngine = llvm::EngineBuilder(TheModule).create();

	/* Initialize the bytecode optimizer */
	TheFPM = new llvm::FunctionPassManager(TheModule);
	TheFPM->add(new llvm::TargetData(*TheExecutionEngine->getTargetData()));
	TheFPM->add(llvm::createPromoteMemoryToRegisterPass());
	TheFPM->add(llvm::createInstructionCombiningPass());
	TheFPM->add(llvm::createReassociatePass());
	TheFPM->add(llvm::createTailCallEliminationPass());
	TheFPM->add(llvm::createGVNPass());
	TheFPM->add(llvm::createCFGSimplificationPass());
	TheFPM->doInitialization();
	
	yyparse();

	/* Now, the AST is in rootnode, we just need to generate LLVM bytecode */
	VariableTree *memroot = new VariableTree();
	for (unsigned int i = 0; i < rootnode->children->size(); i++) {
		(*rootnode->children)[i]->Codegen(memroot);
	}

	std::cerr << "; ----- LLVM Bytecode dump -----" << std::endl;
	TheModule->dump();
	return 0;
}
