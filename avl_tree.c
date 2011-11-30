#include <assert.h>
#include <stdlib.h>
#include "linear_sequence_assoc.h"

#define IS_HANDLE_INVALID(handle)        ((handle) == LSQ_HandleInvalid)

typedef enum {
	BT_AFTERINSERT,
	BT_AFTERDELETE,
}	BalancingTypeT;

typedef enum 
{
	IST_BEFORE_FIRST,
	IST_DEREFERENCABLE,
	IST_PAST_REAR,
} IteratorStateT;

typedef struct TreeNodeStruct
{
	struct TreeNodeStruct * l_child;
	struct TreeNodeStruct * parent;
	struct TreeNodeStruct * r_child;
	int balance;
	int height;
	LSQ_IntegerIndexT key;
	LSQ_BaseTypeT value;
} TreeNodeT;

typedef struct 
{
	TreeNodeT * root;
	int size;
} AVLTreeT;

typedef struct
{
	AVLTreeT * tree;
	TreeNodeT * node;
	IteratorStateT state;
} IteratorT;

static void treeWalkWithDestruction(TreeNodeT * root);
static TreeNodeT * successor(TreeNodeT *node);
static TreeNodeT * predecessor(TreeNodeT * node);
static TreeNodeT * treeMaximum(TreeNodeT * root);
static TreeNodeT * treeMinimum(TreeNodeT * root);
static IteratorT * createIterator(LSQ_HandleT handle, TreeNodeT * node);
static void smallLeftRotate(AVLTreeT *tree, TreeNodeT *root);
static void bigLeftRotate(AVLTreeT *tree, TreeNodeT *root);
static void smallRightRotate(AVLTreeT *tree, TreeNodeT *root);
static void bigRightRotate(AVLTreeT *tree, TreeNodeT *root);
static void restoreBalance(AVLTreeT *tree, TreeNodeT *root, BalancingTypeT balance);
static void replaceNode(AVLTreeT *tree, TreeNodeT *node, TreeNodeT *substitute);
static int treeHeight(const TreeNodeT* root);
static int nodeBalanceFlag(const TreeNodeT* node);
static int maximum(int a, int b);
static void fixTreeHeight(TreeNodeT * root);
static int stopCriterion(BalancingTypeT balance);

static int stopCriterion(BalancingTypeT balance){
	return (int)balance;
}

static int maximum(int a, int b) {
	return a > b ? a : b;
}

static int treeHeight(const TreeNodeT* root) {
	return root != NULL ? root->height : -1;
}

static int nodeBalanceFlag(const TreeNodeT* node) {
	assert(node != NULL);
	return treeHeight(node->l_child) - treeHeight(node->r_child);
}
static void fixTreeHeight(TreeNodeT * root) {
	assert(root != NULL);
	root->height = 1 + maximum(treeHeight(root->l_child), treeHeight(root->r_child));
}

void replaceNode(AVLTreeT *tree, TreeNodeT *node, TreeNodeT *substitute)
{
    if (substitute != NULL)
        substitute->parent = node->parent;

    if (node->parent == NULL)
        tree->root = substitute;
    else
        if (node->parent->l_child == node)
            node->parent->l_child = substitute;
        else
            node->parent->r_child = substitute;
}

static void treeWalkWithDestruction(TreeNodeT * root) 
{
	if (root == NULL)
		return;
	treeWalkWithDestruction(root->r_child);
	treeWalkWithDestruction(root->l_child);
	free(root);
}

static TreeNodeT * successor(TreeNodeT * node)
{
	TreeNodeT * parent = NULL;
	TreeNodeT * cur_node = node;
	if (node == NULL)
		return NULL;
	parent = node->parent;
	if (node->r_child != NULL)
		return treeMinimum(node->r_child);
	while ((parent != NULL) && (cur_node == parent->r_child))
	{
		cur_node = parent;
		parent = parent->parent;
	}
	return parent;
}

static TreeNodeT * predecessor(TreeNodeT * node) 
{
	TreeNodeT * parent = NULL;
	TreeNodeT * cur_node = node;
	if (node == NULL)
		return NULL;
	parent = node->parent;
	if (node->l_child != NULL)
		return treeMaximum(node->l_child);
	while ((parent != NULL) && (cur_node == parent->l_child))
	{
		cur_node = parent;
		parent = parent->parent;
	}
	return parent;
}

static TreeNodeT * treeMaximum(TreeNodeT * root) 
{
	TreeNodeT * max_node = root;
	if (root == NULL)
		return NULL;
	while (max_node->r_child != NULL)
		max_node = max_node->r_child;
	return max_node;
}

static TreeNodeT * treeMinimum(TreeNodeT * root) 
{
	TreeNodeT * min_node = root;
	if (root == NULL)
		return NULL;
	while (min_node->l_child != NULL)
		min_node = min_node->l_child;
	return min_node;
}

static void smallLeftRotate(AVLTreeT *tree, TreeNodeT *root) {
	TreeNodeT * node = NULL;
	if ((root == NULL) || IS_HANDLE_INVALID(tree))
		return;
	node = root->r_child;
	root->r_child = node->l_child;
	if (node->l_child != NULL) 
		node->l_child->parent = root;
	node->parent = root->parent;
	if (root->parent == NULL) 
		tree->root = node;
	else {
		if (root == root->parent->l_child)
			root->parent->l_child = node;
		else
			root->parent->r_child = node;
	}
	node->l_child = root;
	root->parent = node;
	fixTreeHeight(root);
	fixTreeHeight(node);
}

static void smallRightRotate(AVLTreeT *tree, TreeNodeT *root) {
	TreeNodeT * node = NULL;
	if ((root == NULL) || IS_HANDLE_INVALID(tree))
		return;
	node = root->l_child;
	root->l_child = node->r_child;
	if (node->r_child != NULL) 
		node->r_child->parent = root;
	node->parent = root->parent;
	if (root->parent == NULL) 
		tree->root = node;
	else {
		if (root == root->parent->l_child)
			root->parent->l_child = node;
		else
			root->parent->r_child = node;
	}
	node->r_child = root;
	root->parent = node;
	fixTreeHeight(root);
	fixTreeHeight(node);
}

static void bigLeftRotate(AVLTreeT *tree, TreeNodeT *root) {
	if ((root == NULL) || IS_HANDLE_INVALID(tree))
		return;
	smallRightRotate(tree, root->r_child);
	smallLeftRotate(tree, root);
}

static void bigRightRotate(AVLTreeT *tree, TreeNodeT *root) {
	if ((root == NULL) || IS_HANDLE_INVALID(tree))
		return;
	smallLeftRotate(tree, root->l_child);
	smallRightRotate(tree, root);
}

static void restoreBalance(AVLTreeT *tree, TreeNodeT *node, BalancingTypeT balance) {
	TreeNodeT* parent;
    int node_balance, stop_criterion;
	stop_criterion = stopCriterion(balance);

    while (node != NULL)
    {
		fixTreeHeight(node);
        node_balance = nodeBalanceFlag(node);
        parent = node->parent;

        if (abs(node_balance) == stop_criterion)
            return;
        else if (node_balance == -2)
        {
            if (nodeBalanceFlag(node->r_child) > 0)
				smallRightRotate(tree, node->r_child);
			smallLeftRotate(tree, node);
        }
        else if (node_balance == 2)
        {
			if (nodeBalanceFlag(node->l_child) < 0)
                smallLeftRotate(tree, node->l_child);
            smallRightRotate(tree, node);
        }
        node = parent;
    }
}

static IteratorT * createIterator(LSQ_HandleT handle, TreeNodeT * node)
{
	IteratorT * iterator = (IteratorT *)malloc(sizeof(IteratorT));
	if(IS_HANDLE_INVALID(handle)) 
		return LSQ_HandleInvalid;
	if (iterator == NULL) 
		return LSQ_HandleInvalid;
	iterator->tree = (AVLTreeT *)handle;
	iterator->node = node;
	iterator->state = node != NULL ? IST_DEREFERENCABLE : IST_PAST_REAR;
	return iterator;
}

extern LSQ_HandleT LSQ_CreateSequence(void) 
{
	AVLTreeT * tree = (AVLTreeT *)malloc(sizeof(AVLTreeT));
	if (tree == NULL)
		return LSQ_HandleInvalid;
	tree->size = 0;
	tree->root = NULL;
	return tree;
}

extern void LSQ_DestroySequence(LSQ_HandleT handle) 
{
	AVLTreeT * tree = (AVLTreeT *)handle;
	if IS_HANDLE_INVALID(handle)
		return;
	treeWalkWithDestruction(tree->root);
	free(tree);
}

extern LSQ_IntegerIndexT LSQ_GetSize(LSQ_HandleT handle)
{  
	return IS_HANDLE_INVALID(handle) ? -1 : ((AVLTreeT *)handle)->size;
}

extern int LSQ_IsIteratorDereferencable(LSQ_IteratorT iterator)
{
	IteratorT * iter = (IteratorT *)iterator;
	return !IS_HANDLE_INVALID(iterator) && iter->state == IST_DEREFERENCABLE;
}

extern int LSQ_IsIteratorPastRear(LSQ_IteratorT iterator)
{	
	IteratorT * iter = (IteratorT *)iterator;
	return !IS_HANDLE_INVALID(iterator) && iter->state == IST_PAST_REAR;
}

extern int LSQ_IsIteratorBeforeFirst(LSQ_IteratorT iterator)
{
	IteratorT * iter = (IteratorT *)iterator;
	return !IS_HANDLE_INVALID(iterator) && iter->state == IST_BEFORE_FIRST;
}

extern LSQ_BaseTypeT* LSQ_DereferenceIterator(LSQ_IteratorT iterator)
{
	IteratorT * iter = (IteratorT *)iterator;
	return !LSQ_IsIteratorDereferencable(iterator) ? NULL :
		&iter->node->value;
}

extern LSQ_IntegerIndexT LSQ_GetIteratorKey(LSQ_IteratorT iterator) {
	IteratorT * iter = (IteratorT *)iterator;
	assert(LSQ_IsIteratorDereferencable(iterator));
	return iter->node->key;
}

extern LSQ_IteratorT LSQ_GetElementByIndex(LSQ_HandleT handle, LSQ_IntegerIndexT index)
{
	AVLTreeT * tree = (AVLTreeT *)handle;
	TreeNodeT * node = tree->root;
	if IS_HANDLE_INVALID(handle)  
		return LSQ_HandleInvalid;
	while ((node != NULL) && (node->key != index)) 
		node = (index > node->key) ? node->r_child : node->l_child;
	return createIterator(handle, node);
}

extern LSQ_IteratorT LSQ_GetFrontElement(LSQ_HandleT handle)
{
	AVLTreeT * tree = (AVLTreeT *)handle;
	if IS_HANDLE_INVALID(handle)
		return LSQ_HandleInvalid;
	return createIterator(handle, treeMinimum(tree->root));
}

extern LSQ_IteratorT LSQ_GetPastRearElement(LSQ_HandleT handle)
{
	if IS_HANDLE_INVALID(handle)
		return LSQ_HandleInvalid;
	return createIterator(handle, NULL);	
}

extern void LSQ_DestroyIterator(LSQ_IteratorT iterator)
{
	if (IS_HANDLE_INVALID(iterator))  
		return;
	free(iterator);
}

extern void LSQ_AdvanceOneElement(LSQ_IteratorT iterator)
{
	IteratorT * iter = (IteratorT *)iterator;
	if (IS_HANDLE_INVALID(iterator) || (iter->tree->size == 0))
		return;
	if (LSQ_IsIteratorBeforeFirst(iterator)){
		iter->node = treeMinimum(iter->tree->root);	
		iter->state = IST_DEREFERENCABLE;
		return;
	}
	iter->node = successor(iter->node);
	if (iter->node == NULL)
		iter->state = IST_PAST_REAR;
}

extern void LSQ_RewindOneElement(LSQ_IteratorT iterator)
{
	IteratorT * iter = (IteratorT *)iterator;
	if (IS_HANDLE_INVALID(iterator) || (iter->tree->size == 0))
		return;
	if (LSQ_IsIteratorPastRear(iterator)){
		iter->node = treeMaximum(iter->tree->root);	
		iter->state = IST_DEREFERENCABLE;
		return;
	}
	iter->node = predecessor(iter->node);
	if (iter->node == NULL)
		iter->state = IST_BEFORE_FIRST;
}

extern void LSQ_ShiftPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT shift)
{
	IteratorT * iter = (IteratorT *)iterator;
	if IS_HANDLE_INVALID(iterator)
		return;
	for(; shift > 0; shift--)
		LSQ_AdvanceOneElement(iterator);
	for(; shift < 0; shift++)
		LSQ_RewindOneElement(iterator);
}

extern void LSQ_SetPosition(LSQ_IteratorT iterator, LSQ_IntegerIndexT pos)
{
	IteratorT * iter = (IteratorT *)iterator;
	if IS_HANDLE_INVALID(iterator)
		return;
	iter->state = IST_BEFORE_FIRST;
	LSQ_ShiftPosition(iterator, pos + 1);
}

extern void LSQ_InsertElement(LSQ_HandleT handle, LSQ_IntegerIndexT key, LSQ_BaseTypeT value)
{
	AVLTreeT *tree = (AVLTreeT *)handle;
	TreeNodeT *insert_node = NULL, 
			  *node = NULL, 
			  *parent = NULL;
	if (IS_HANDLE_INVALID(handle))
        return;
	node = tree->root;
	while (node != NULL) 
	{
		parent = node;
		if (key > node->key)
			node = node->r_child; 
		else if (key < node->key)
			node = node->l_child;
		else {
			node->value = value;
			return;
		}
	}
	insert_node = (TreeNodeT *)malloc(sizeof(TreeNodeT));
	if (insert_node == NULL)
		return;
	insert_node->key = key;
	insert_node->value = value;
	insert_node->r_child = NULL;
	insert_node->l_child = NULL;
	tree->size++;
	insert_node->parent = parent; 
	if (parent == NULL) 
	{
		tree->root = insert_node;
		return;
	}
	if (key > parent->key)
		parent->r_child = insert_node;
	else
		parent->l_child = insert_node;
	restoreBalance(tree, parent, BT_AFTERINSERT);
}

extern void LSQ_DeleteFrontElement(LSQ_HandleT handle)
{
	IteratorT *iterator = (IteratorT *)LSQ_GetFrontElement(handle);
	LSQ_DeleteElement(handle, iterator->node->key);
	LSQ_DestroyIterator(iterator);
}

extern void LSQ_DeleteRearElement(LSQ_HandleT handle)
{
	IteratorT *iterator = (IteratorT *)LSQ_GetPastRearElement(handle);
	LSQ_RewindOneElement(iterator);
	LSQ_DeleteElement(handle, iterator->node->key);
	LSQ_DestroyIterator(iterator);
}

extern void LSQ_DeleteElement(LSQ_HandleT handle, LSQ_IntegerIndexT key) 
{
	AVLTreeT *tree = (AVLTreeT *)handle;
	TreeNodeT *node = NULL, *parent = NULL;
	IteratorT * iter = (IteratorT *)LSQ_GetElementByIndex(handle, key);
    int new_key;
    if (!LSQ_IsIteratorDereferencable(iter))
        return;

    parent = iter->node->parent;
    if (iter->node->l_child == NULL && iter->node->r_child == NULL)
        replaceNode(tree, iter->node, NULL);
    else if (iter->node->l_child != NULL && iter->node->r_child != NULL)
    {
        node = successor(iter->node);
        new_key = node->key;
        iter->node->value = node->value;
        LSQ_DeleteElement(handle, node->key);
        iter->node->key = new_key;
        return;
    }
    else if (iter->node->l_child != NULL)
        replaceNode(tree, iter->node, iter->node->l_child);
    else if (iter->node->r_child != NULL)
        replaceNode(tree, iter->node, iter->node->r_child);
    free(iter->node);
    tree->size--;
	restoreBalance(tree, parent, BT_AFTERDELETE);
}