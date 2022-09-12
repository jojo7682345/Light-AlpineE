#include "tree.h"
#include <MemoryUtilities/MemoryUtilities.h>

void nodeAddChild(TreeNode* node, TreeNode* child) {
	if (child->parent) {
		return;
	}
	if (node->children == NULL) {
		node->children = fsAllocate(sizeof(TreeNode*));
		node->children[0] = child;
		node->childCount = 1;
	} else {
		node->children = fsReallocate(node->children, sizeof(TreeNode*) * (node->childCount + 1));
		node->children[node->childCount] = child;
		node->childCount++;
	}
	child->parent = node;
}

