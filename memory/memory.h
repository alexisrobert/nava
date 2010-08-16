#include <map>
#include <llvm/Value.h>

class VariableTree {
	public:
		VariableTree(VariableTree* parent = 0);
		~VariableTree();

		void set(std::string &name, llvm::Value* value);
		llvm::Value* get(std::string &name);
	
	protected:
		std::map<std::string, llvm::Value*> *values;
		VariableTree* parent;
};
