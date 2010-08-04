#include <map>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/PassManager.h>

extern llvm::Module *TheModule;
extern llvm::ExecutionEngine *TheExecutionEngine;
extern llvm::FunctionPassManager *TheFPM;

extern llvm::IRBuilder<> Builder;
extern std::map<std::string, llvm::Value*> NamedValues;
