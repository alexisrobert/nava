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

#include "memory.h"
#include <llvm/Support/IRBuilder.h>
#include <llvm/LLVMContext.h>

VariableLeaf::VariableLeaf() {
	this->value = 0;
}

VariableLeaf::~VariableLeaf() {
	if (this->value != 0)
		delete this->value;
}

VariableTree::VariableTree(VariableTree *parent) {
	this->parent = parent;
	this->values = new std::map<std::string, VariableLeaf*>();
}

VariableTree::~VariableTree() {
	delete this->values;
}

void VariableTree::set(std::string name, llvm::AllocaInst* value) {
	VariableLeaf *leaf = new VariableLeaf();
	leaf->value = value;

	(*values)[name] = leaf;
}

llvm::AllocaInst* VariableTree::get(std::string &name) {
	std::map<std::string, VariableLeaf*>::iterator it = this->values->begin();
	VariableLeaf *var = 0;

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
		return var->value;
	}
}

llvm::AllocaInst* VariableTree::CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName, const llvm::Type *type) {
	llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());

	return TmpB.CreateAlloca(type, 0, VarName.c_str());
}
