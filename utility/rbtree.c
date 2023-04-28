#include <stdio.h>
#include <stdlib.h>
#include "rbtree.h"

// #define RED		0
// #define BLACK	1




#define swap(a ,b)	{a = a^b; b = a^b; a = a^b;}

#if 0
static void set_parent(rbtree *parent, rbtree_node *node) {
	rbtree_node *parent = node->parent;
	if (parent == T->nil) {
		T->root = node;
	} else if (node == parent->left) {
		parent->left = child;
	} else {
		parent->right = child;
	}
}
#endif

// 只可能被情况1or2 使用
static int logic_delete(rbtree *T, rbtree_node *node, rbtree_node *child) {
	if (T == NULL) return -1;
	if (node == NULL || node == T->nil) return -1;
	if (child == NULL) return -1;

	rbtree_node *parent = node->parent;

	if (child == T->nil) {
		child->parent = parent;
	}

	if (parent == T->nil) {
		T->root = child;
	} else if (node == parent->left) {
		parent->left = child;
	} else {
		parent->right = child;
	}
	return 0;
}



// 该结点变为右孩子结点的左孩子，结点 -> 左孩子
static void left_rotate(rbtree *T, rbtree_node *parent) {
	rbtree_node *rchild = parent->right;

	// parent -- rchild
	parent->right = rchild->left;
	if (rchild->left!=T->nil) {
		(rchild->left)->parent = parent;
	}

	// parent
	rchild->parent = parent->parent;
	if (parent->parent==T->nil){	// update root
		T->root = rchild;
	} else if (parent == parent->parent->left){
		parent->parent->left = rchild;
	} else {
		parent->parent->right = rchild;
	}

	// branch
	rchild->left = parent;
	parent->parent = rchild;
}


static void right_rotate(rbtree *T, rbtree_node *y) {
	rbtree_node *x = y->right;

	// x -- y
	y->left = x->right;
	if (x->right!=T->nil) {
		(x->right)->parent = y;
	}

	// parent
	x->parent = y->parent;
	if (y->parent==T->nil){	// update root
		T->root = x;
	} else if (y == y->parent->right){
		y->parent->right = x;
	} else {
		y->parent->left = x;
	}

	// branch
	x->left = y;
	y->parent = x;
}

KEY_TYPE key_compare(const KEY_TYPE *x, const KEY_TYPE *y) {
	;
}


/*
	@brief 搜索节点
	@return 成功返回节点，失败返回NULL
*/
rbtree_node* rbtree_find(rbtree *T, const KEY_TYPE key) {
	rbtree_node *node = T->root;
	if (node == T->nil) return NULL;

	while (node != T->nil && node->key != key) {
		if (node->key < key) {
			node = node->right;
		} else if (node->key > key) {
			node = node->left;
		}
	}
	if (node == T->nil) return NULL;
	return node;

}


static void insert_fixup(rbtree *T, rbtree_node *node){
	if (node->parent != RED) return;
	// 新增结点是父结点的左孩子
	// 需要不断往上进行处理，该结点为“红色”
	rbtree_node *parent = node->parent, *uncle,*pparent = parent->parent;
	if (node->parent == node->parent->parent->left) {	// 确定节点 parent 是左子树
		while (node->parent->color == RED) {
			uncle = parent->right;

			if (uncle->color == RED) {		// the color of parent and uncle are the same
				parent->color = uncle->color = BLACK;
				pparent->color = RED;
				// 继续向上遍历
				node = pparent;
				parent = node->parent;
				pparent = parent->parent;
			} else {
				// 新增结点是父结点的右孩子，进行左单旋
				if (node == node->parent->right) {
					node = node->parent;
					left_rotate(T, node);
				}
				parent->color = BLACK;
				pparent->color = RED;
				right_rotate(T, pparent);
			}
		}
	} else {
		while (node->parent->color == RED) {
			uncle = pparent->left;

			if (uncle->color == RED) {
				node->color = RED;
				parent->color = uncle->color = BLACK;
				pparent->color = RED;
				// 继续向上遍历
				node = pparent;
				parent = node->parent;
				pparent = parent->parent;
			} else {
				if (node == parent->left) {
					node = parent;
					right_rotate(T, node);
				}
				parent->color = BLACK;
				pparent->color = RED;
				left_rotate(T, pparent);
			}
		}
	}

	// 可能调整的过程中导致根结点，变为“红色”
	T->root->color = BLACK;
}


void rbtree_insert(rbtree *T, rbtree_node *new_node) {
	rbtree_node *tmp = T->root;
	rbtree_node *parent = T->nil;
	while (tmp != NULL) {
		parent = tmp;	//parent 是 tmp 的父结点
		if (new_node->key < tmp->key) {
			tmp = tmp->left;
		} else if (new_node->key > tmp->key) {
			tmp = tmp->right;
		} else {
			// 根据业务设置
			/* DOTO */
			return;
		}
	}
	new_node->parent = parent;
	if (parent == T->nil) {	// 更新root
		T->root = new_node;
	} else if (new_node->key < parent->key) {
		parent->left = new_node;
	} else {
		parent->right = new_node;
	}

	new_node->left = T->nil;
	new_node->right = T->nil;
	new_node->color = RED;

	insert_fixup(T, new_node);
	return;
}


/*
	@brief 寻找节点后继
*/
static rbtree_node* find_successor(rbtree *T, rbtree_node *node) {
	if (T == NULL || node == NULL) return NULL;

	rbtree_node *parent = node->parent;

	// 存在下后继
	if (node->right != NULL) {
		while (node->left != T->nil) {
			node = node->left;
		}
	} else {	// 向上寻找后继
		while (node != parent->left && parent != T->nil) {
			node = parent;
			parent = parent->parent;
		}
		if (parent == T->nil) return NULL;	// 不存在后继
		node = parent;		// 查找成功
	}
	return node;
}



static void delete_fixup(rbtree *T, rbtree_node *node) {
	if (T == NULL || node==NULL || node == T->nil) return;

	// 情形2-只有一个孩子-->此节点必为“黑色”节点，孩子必为“红色” && 孩子节点没有孩子
	if (node->left != T->nil && node->right == T->nil) {
		node->left->color = BLACK;
	} else if (node->right != T->nil && node->left == T->nil) {
		node->right->color = BLACK;
	} else {
		;
	}
}


void rbtree_delete(rbtree *T, const KEY_TYPE key) {
	rbtree_node *node = rbtree_find(T, key);
	if (node == NULL) return;

// again:
	int again = 0;
do{
	rbtree_node *child = NULL;
	rbtree_node *parent = node->parent;
	// 情形1：node 没有子节点
	if (node->left == T->nil && node->right == T->nil) {
		logic_delete(T, node, node->left);
		delete_fixup(T, node);
	}
	// 情形2：node 只有一个节点
	else if (node->left != T->nil && node->right == T->nil) {
		child = node->left;
		logic_delete(T, node, child);
		delete_fixup(T, node);
	} else if (node->right != T->nil && node->left == T->nil) {
		child = node->right;
		logic_delete(T, node, child);
		delete_fixup(T, node);
	}
	// 情形3： node 有两个节点
	else /*if (node->left != T->nil && node->right != T->nil)*/ {
		rbtree_node *successor = find_successor(T, node);
		// if (successor == NULL) return; 情形3 一定有后继

		swap(successor->key, node->key);
		rbtree_node *tmp = successor->value;
		successor->value = node->value;
		node->value = successor->value;

		node = successor;
		again = 1;		// 情形3转换为情形1或2，回到上面代码
	}
} while(again-- > 0);

	free(node); node = NULL;

	// 删除节点后，进行调整
	rbtree_delete_fixup(T, node);
}
