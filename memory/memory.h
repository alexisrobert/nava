#include <map>
#include <llvm/Instructions.h>
#include <llvm/Function.h>

enum _VariableType {
	INTEGER = 0,
	DOUBLE = 1
};

typedef enum _VariableType VariableType;

class VariableLeaf {
	public:
		VariableLeaf();
		~VariableLeaf();

		llvm::AllocaInst *value;
		VariableType type;
};

class VariableTree {
	public:
		VariableTree(VariableTree* parent = 0);
		~VariableTree();

		void set(std::string name, VariableType type, llvm::AllocaInst* value);
		llvm::AllocaInst* get(std::string &name, VariableType type);

		static llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function *TheFunction, const std::string &VarName);
	
	protected:
		std::map<std::string, VariableLeaf*> *values;
		VariableTree* parent;
};
