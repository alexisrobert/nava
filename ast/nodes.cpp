#include "nodes.h"

using namespace llvm;

Value *IntegerExprAST::Codegen() {
	return ConstantFP::get(getGlobalContext(), APFloat((float)Val));
}
