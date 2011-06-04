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

#include "nodes.h"
#include "globals.h"
#include "../parser.hpp"

#include <llvm/Support/TypeBuilder.h>

using namespace llvm;

VariableDefAST::VariableDefAST(VariableExprAST *lhs, ExprAST *rhs, int type) {
	this->LHS = lhs;
	this->RHS = rhs;

	this->Type = getTypeFromAST(type);
}

const llvm::Type* VariableDefAST::getTypeFromAST(int type) {
	switch (type) {
	case TDOUBLE:
		return llvm::TypeBuilder<llvm::types::ieee_double, true>().get(llvm::getGlobalContext()); break;
	case TINTEGER:
		return llvm::TypeBuilder<int, false>().get(llvm::getGlobalContext()); break;
	default:
		std::cerr << "Unknown type." << std::endl;
		return llvm::TypeBuilder<void, true>().get(llvm::getGlobalContext()); break;
	}
}

Value *VariableAssignAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Fetch the alloca
	L = memctx->get(LHS->getName());
	if (L == 0) return ErrorV("Variable not found.");

	// Check if R is the same type of L
	if (R->getType()->getPointerTo() != L->getType()) return ErrorV("Different type assignment.");

	// Store the content
	Builder.CreateStore(R, L);
	return R;
}

Value *VariableDefAST::Codegen(VariableTree *memctx) {
	// Fetching the value
	Value *R = RHS->Codegen(memctx);
	if (R == 0) return 0;

	AllocaInst *L;

	// Create the alloca
	L = Builder.CreateAlloca(R->getType(), 0, LHS->getName().c_str());

	// TODO: Check the type !!!
	
	memctx->set(LHS->getName(), L);

	// Store the content
	Builder.CreateStore(R, L);

	return R;
}
