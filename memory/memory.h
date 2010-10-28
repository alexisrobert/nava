#include <map>
#include <llvm/Instructions.h>
#include <llvm/Function.h>

class VariableTree {
	public:
		VariableTree(VariableTree* parent = 0);
		~VariableTree();

		void set(std::string name, llvm::AllocaInst* value);
		llvm::AllocaInst* get(std::string &name);

		static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName);
	
	protected:
		std::map<std::string, llvm::AllocaInst*> *values;
		VariableTree* parent;
};
