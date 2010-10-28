#include "memory.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/LLVMContext.h>

VariableTree::VariableTree(VariableTree *parent) {
	this->parent = parent;
	this->values = new std::map<std::string, llvm::AllocaInst*>();
}

VariableTree::~VariableTree() {
	delete this->values;
}

void VariableTree::set(std::string name, llvm::AllocaInst* value) {
	(*values)[name] = value;
}

llvm::AllocaInst* VariableTree::get(std::string &name) {
	std::map<std::string, llvm::AllocaInst*>::iterator it = this->values->begin();
	llvm::AllocaInst *var = 0;

	while (it != this->values->end()) {
		if ((*it).first == name) {
			var = (*it).second;
			break;
		}

		it++;
	}

	if (var == 0) {
		if (parent == 0) {
			return NULL;
		} else {
			// If the key is not present, check if the parent has the requested symbol name
			return parent->get(name);
		}
	} else {
		return var;
	}
}

llvm::AllocaInst* VariableTree::CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName) {
	llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());

	return TmpB.CreateAlloca(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 0, VarName.c_str());
}
