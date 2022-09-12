#pragma once
#include <dataTypes.h>

typedef struct TreeNode TreeNode;

typedef struct TreeNode {
	void* data;
	
	TreeNode* parent;
	uint32_t childCount;
	TreeNode** children;
}TreeNode;


void nodeAddChild(TreeNode* node, TreeNode* child);
