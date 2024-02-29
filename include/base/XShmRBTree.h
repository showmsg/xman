/**  
 * @file: XShmRBTree.h
 * @Description: TODO(描述)
 * @author kangyq
 * @date 2022-07-14 09:46:40 
 */
#ifndef INCLUDE_BASE_XSHMRBTREE_H_
#define INCLUDE_BASE_XSHMRBTREE_H_

#include <stdlib.h>
#include <stdint.h>
#include "XShmLib.h"

typedef enum {
	RED, BLACK,
} XRBColor;

typedef enum {
	LEFT, RIGHT,
} XRBLeaf;


typedef struct XRBTree {
	struct XRBTree* left;
	struct XRBTree* right;
	struct XRBTree* parent;
	struct XRBTree* root;
	XLLong key;
	XRBColor color;
	XLLong data;
}XRBTreeT;

typedef struct XRBRoot {
	XRBTreeT *tree;
	size_t size;
}XRBRootT;


extern int XShmTreeInsert(XRBRootT *root, XRBTreeT *node);

extern XRBTreeT* XShmTreeSearch(XRBRootT *root, int key);

extern XRBTreeT* XShmTreeFindRightMost(XRBTreeT *tree);

extern XRBTreeT* XShmTreeFindLeftMost(XRBTreeT *tree);

extern int XShmTreeDeleteNode(XRBRootT *root, XRBTreeT *node);

extern void XShmTreeDump(XRBTreeT *tree, FILE *stream);

extern void XShmTreeAddNode(XRBRootT *root, int key, XLLong data);

extern void XShmTreeCleanup(XRBTreeT *root);

#endif /* INCLUDE_BASE_XSHMRBTREE_H_ */
