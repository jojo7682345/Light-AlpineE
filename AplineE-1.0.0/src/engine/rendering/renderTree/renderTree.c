#include "renderTree.h"
#include <util/tree.h>
#include <core/core.h>
#include <math.h>

//void renderTreeBuildBranch(TreeNode* node, uint32_t nodeCount, TreeNode* tree) {
//	RenderSurfaceCreateInfo* renderSurfaceCreateInfo = (RenderSurfaceCreateInfo*)node->data;
//	uint32_t dependencyCount = renderSurfaceCreateInfo->dependencyCount;
//	for (uint32_t i = 0; i < dependencyCount; i++) {
//		RenderSurface* dependency = renderSurfaceCreateInfo->dependencies[i];
//		for (uint32_t j = 0; j < nodeCount; j++) {
//			TreeNode* otherNode = tree + j;
//			RenderSurfaceCreateInfo* renderSurfaceCreateInfo = (RenderSurfaceCreateInfo*)otherNode->data;
//			RenderSurface* renderSurface = renderSurfaceCreateInfo->renderSurface;
//			if (renderSurface == dependency) {
//				nodeAddChild(node, otherNode);
//				renderTreeBuildBranch(otherNode, nodeCount, tree);
//				break;
//			}
//		}
//	}
//}
//
//void countPermutations(TreeNode* node, uint32_t* permutationCount) {
//	RenderSurfaceCreateInfo* data = (RenderSurfaceCreateInfo*)node->data;
//	if (data->unstable) {
//		(*permutationCount)++;
//	}
//	uint32_t childPermutationCount = 1;
//	for (uint32_t i = 0; i < node->childCount; i++) {
//		TreeNode* child = node->children[i];
//		uint32_t cPc = 0;
//		countPermutations(child, &cPc);
//		childPermutationCount *= cPc;
//	}
//	*permutationCount += childPermutationCount;
//}
//
//
//void modifyNodes(TreeNode* node, bool_t* treeIndex, uint32_t permutationIndex, uint32_t* nodeIndex) {
//
//
//
//	for (uint32_t i = 0; i < node->childCount; i++) {
//
//		TreeNode* child = node->children[i];
//
//		RenderSurfaceCreateInfo childInfo = *(RenderSurfaceCreateInfo*)child->data;
//
//		if (childInfo.unstable) {
//			uint32_t bitMask = 1 << (*nodeIndex);
//
//			if (!(permutationIndex & bitMask)) {
//				node->children[i] = nullptr;
//			}
//		}
//
//		modifyNodes(child, treeIndex, permutationIndex, nodeIndex);
//	}
//
//	nodeIndex++;
//
//}
//
//void modifyTree(TreeNode* tree, bool_t* treeIndex, uint32_t permutationIndex, uint32_t permutation) {
//
//
//	uint32_t nodeIndex = 0;
//	modifyNodes(tree, treeIndex, permutationIndex, &nodeIndex);
//
//}
//
//
//void renderTreesBuild(EngineHandle handle) {
//
//	RenderSurfaceCreationPool renderSurfaces = buildRenderSurfaces(
//		handle,
//		renderSurfaceGetDefaultApplicationSurfaceSettings(),
//		renderSurfaceGetDefaultApplicationSurfaceUiSettings()
//	);
//
//	RenderSurfaceCreateInfo appSurfCreateInfo = convertApplicationSurfaceSettingsToSurfaceCreateInfo(handle, renderSurfaces.applicationSurfaceSettings);
//	RenderSurfaceCreateInfo appSurfUiCreateInfo = convertApplicationSurfaceUiSettingsToSurfaceCreateInfo(handle, renderSurfaces.applicationSurfaceUiSettings);
//
//	//construct default render surface dependency tree
//	uint32_t surfaceCount = renderSurfaces.renderSurfaceCreateCount + 2;
//	TreeNode* tree = (TreeNode*)fsAllocate(sizeof(TreeNode) * (surfaceCount));
//
//	//add all render surfaces to the tree
//	tree[0].data = &appSurfCreateInfo;
//	tree[1].data = &appSurfUiCreateInfo;
//	for (uint32_t i = 2; i < surfaceCount; i++) {
//		tree[i].data = renderSurfaces.renderSurfaceCreateInfos + i - 2;
//	}
//
//	renderTreeBuildBranch(tree, surfaceCount, tree);
//
//	////find number of permutations;
//	//uint32_t permutationCount = 0;
//	//countPermutations(tree, &permutationCount);
//
//	////create every permutation
//	//TreeNode** permutations = (TreeNode**)fsAllocate(sizeof(TreeNode*) * permutationCount);
//	//bool_t** permutationIndices = (bool_t**)fsAllocate(sizeof(bool_t*) * permutationCount);
//
//	//for (uint32_t i = 0; i < permutationCount; i++) {
//	//	TreeNode* permutationTree = (TreeNode*)fsAllocate(sizeof(TreeNode) * surfaceCount);
//	//	memcpy(permutationTree, tree, sizeof(TreeNode) * surfaceCount);
//
//	//	bool_t* permutationIndex = (bool_t*)fsAllocate(sizeof(bool_t) * surfaceCount);
//
//	//	permutations[i] = permutationTree;
//	//	permutationIndices[i] = permutationIndex;
//	//}
//	//for (uint32_t i = 0; i < permutationCount; i++) {
//	//	modifyTree(permutations[i], permutationIndices[i], ~i, i);
//	//}
//
//	
//
//	//create renderPass
//	
//
//	
//	renderSurfaces.releaseCallback(renderSurfaces.renderSurfaceCreateInfos);
//	fsFree(tree);
//}
