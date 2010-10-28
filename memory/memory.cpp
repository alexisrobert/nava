#include "memory.h"

VariableTree::VariableTree(VariableTree *parent) {
	this->parent = parent;
	this->values = new std::map<std::string, llvm::Value*>();
}

VariableTree::~VariableTree() {
	delete this->values;
}

void VariableTree::set(std::string name, llvm::Value* value) {
	(*values)[name] = value;
}

llvm::Value* VariableTree::get(std::string &name) {
	std::map<std::string, llvm::Value*>::iterator it = this->values->begin();
	llvm::Value *var = 0;

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
