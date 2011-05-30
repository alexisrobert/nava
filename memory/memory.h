#include <map>
#include <llvm/Instructions.h>
#include <llvm/Function.h>

// TODO: Remove this useless class.
class VariableLeaf {
	public:
		VariableLeaf();
		VariableLeaf(llvm::AllocaInst *v);
		~VariableLeaf();

		llvm::AllocaInst *value;
};

class VariableTree {
	public:
		VariableTree(VariableTree* parent = 0);
		~VariableTree();

		void set(std::string name, llvm::AllocaInst* value);
		llvm::AllocaInst* get(std::string &name);

		static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName);
	
	protected:
		std::map<std::string, VariableLeaf*> *values;
		VariableTree* parent;
};
