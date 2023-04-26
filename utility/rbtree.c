#include <stdio.h>
#include <stdlib.h>

#define RED		0
#define BLACK	1

typedef int KEY_TYPE;	// Key type

typedef struct _rb_node{
	unsigned char color;
	struct _rb_node *left;
	struct _rb_node *right;
	struct _rb_node *parent;	// 用于旋转操作
	KEY_TYPE key;
	void *data;
} rbtree_node;

typedef struct _rbtree {
	struct _rb_node *root;
	struct _rb_node *nil;	// 通用结点
} rbtree;

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

KEY_TYPE key_compare(const KEY_TYPE *x, const KEY_TYPE *y){
	;
}


static void rbtree_insert_fixup(rbtree *T, rbtree_node *node){
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

	rbtree_insert_fixup(T, new_node);
	return;
}

void rbtree_delete(rbtree *T, rbtree_node *node) {

}
