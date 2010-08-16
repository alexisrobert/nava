#include "memory.h"

VariableTree::VariableTree(VariableTree *parent) {
	this->parent = parent;
	this->values = new std::map<std::string, llvm::Value*>();
}

VariableTree::~VariableTree() {
	delete this->values;
}

void VariableTree::set(std::string &name, llvm::Value* value) {
	(*values)[name] = value;
}

llvm::Value* VariableTree::get(std::string &name) {
	// If the key is not present, check if the parent has the requested symbol name
	if (values->find(name) == values->end()) {
		if (parent == 0) {
			return NULL;
		} else {
			return parent->get(name);
		}
	} else {
		return (*values)[name];
	}
}
