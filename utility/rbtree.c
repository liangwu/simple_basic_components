#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "rbtree.h"


#define DEBUG		0		// 1: enable debug | 0: disable debug

// 交换 a 与 b 的数据，但不能用于指针类型
#define swap(a ,b)	{a = a^b; b = a^b; a = a^b;}

#if DEBUG
	#define RB_DEBUG(INFO)		printf("DEBUG: %s in line %d\n", INFO, __LINE__)
	#define RB_DEBUG_INT(INT)	printf("INT: %d in line %d\n", INT, __LINE__)
#else
	#define RB_DEBUG(INFO)
	#define RB_DEBUG_INT(INT)
#endif // !DEBUG



typedef enum color{
	RED,
	BLACK
} color;


static inline void rb_set_color(rbtree_node *node, color c) {
	node->color = c;
}

static inline bool rb_node_is_red(rbtree_node *node) {
	return node->color == RED ? true : false;
}

static inline bool rb_cmp_black(rbtree_node *node) {
	return node->color == BLACK;
}

static inline void rb_node_init(rbtree *T, rbtree_node *node, rbtree_node *parent) {
	node->parent = parent;
	node->left = node->right = T->nil;
	rb_set_color(node, RED);
}


rbtree_node* rb_node_create(KEY_TYPE key, void* value) {
	//if (value == NULL) return NULL;

	rbtree_node *node = calloc(1, sizeof(rbtree_node));
	if (node == NULL) {
		printf("Error in %s, line: %d\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	node->parent = node->left = node->right = NULL;
	node->key = key;
	node->value = value;
}



// 只可能被情况1or2 使用
static int logic_delete(rbtree *T, rbtree_node *node, rbtree_node *child) {
	if (T == NULL) return -1;
	if (node == NULL || node == T->nil) return -1;
	if (child == NULL) return -1;

	rbtree_node *parent = node->parent;

	if (child != T->nil) {
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
	swap(rchild->color, parent->color);
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


static void right_rotate(rbtree *T, rbtree_node *parent) {
	rbtree_node *child = parent->left;
	swap(child->color, parent->color);
	// child -- parent
	parent->left = child->right;
	if (child->right!=T->nil) {
		(child->right)->parent = parent;
	}

	// parent
	child->parent = parent->parent;
	if (parent->parent==T->nil){	// update root
		T->root = child;
	} else if (parent == parent->parent->right){
		parent->parent->right = child;
	} else {
		parent->parent->left = child;
	}

	// branch
	child->right = parent;
	parent->parent = child;
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
	if (node->parent->color == BLACK) return;
	// 新增结点是父结点的左孩子
	// 需要不断往上进行处理，该结点为“红色”
	rbtree_node *parent, *uncle,*pparent;

	while (rb_node_is_red(node->parent)) {
		parent = node->parent;
		pparent = parent->parent;
		if (parent == pparent->left) {	// 确定节点 parent 是左子树

			RB_DEBUG("确定节点 parent 是左子树");
			uncle = pparent->right;

			if (rb_node_is_red(uncle)) {		// the color of parent and uncle are the same
				RB_DEBUG("uncle is red");
				RB_DEBUG("swap(parent->color, pparent->color)");
				swap(parent->color, pparent->color);
				uncle->color = parent->color;
				// 继续向上遍历
				node = pparent;
			} else {
				RB_DEBUG("uncle is black");
				// 新增结点是父结点的右孩子，进行左单旋
				if (node == node->parent->right) {
					node = node->parent;
					RB_DEBUG("left_rotate");
					left_rotate(T, node);
				}
				RB_DEBUG("right_rotate");
				right_rotate(T, pparent);
			}

		} else {	// "确定节点 parent 是右子树"

			RB_DEBUG("确定节点 parent 是右子树");
			uncle = pparent->left;

			if (rb_node_is_red(uncle)) {
				RB_DEBUG("uncle is red");
				RB_DEBUG("swap(parent->color, pparent->color)");
				swap(parent->color, pparent->color);
				uncle->color = parent->color;
				node = pparent;		// 继续向上遍历
			} else {
				RB_DEBUG("uncle is black");
				if (node == parent->left) {
					node = parent;
					RB_DEBUG("right_rotate");
					right_rotate(T, node);
				}
				RB_DEBUG("left_rotate");
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
	while (tmp != T->nil) {
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

	rb_node_init(T, new_node, parent);

	if (parent == T->nil) {	// 更新root
		T->root = new_node;
		rb_set_color(new_node, BLACK);
		RB_DEBUG("root");
	} else if (new_node->key < parent->key) {
		parent->left = new_node;
		RB_DEBUG("left");
	} else {
		parent->right = new_node;
		RB_DEBUG("right");
	}

	RB_DEBUG_INT(new_node->key);
	insert_fixup(T, new_node);
	return;
}


/*
	@brief 寻找节点后继
*/
static rbtree_node* find_successor(rbtree *T, rbtree_node *node) {
	if (T == NULL || node == NULL) return NULL;

	// 存在下后继
	if (node->right != T->nil) {
		node = node->right;
		while (node->left != T->nil) {
			node = node->left;
		}
	} else {	// 向上寻找后继
		rbtree_node *parent = node->parent;
		while (node != parent->left && parent != T->nil) {
			node = parent;
			parent = parent->parent;
		}
		if (parent == T->nil) return NULL;	// 不存在后继
		node = parent;	// 成功找到
	}
	return node;
}



static void delete_fixup(rbtree *T, rbtree_node *node) {
	if (T == NULL || node==NULL || node == T->nil) return;

again:
	// 情形2-只有一个孩子-->此节点必为“黑色”节点，孩子必为“红色” && 孩子节点没有孩子
	if (node->left != T->nil && node->right == T->nil) {
		node->left->color = BLACK;
	} else if (node->left == T->nil && node->right != T->nil) {
		node->right->color = BLACK;
	} else {
		// 情形1-删除节点无孩子节点
		if (node->color == RED) return;		// “红色”节点无需调整

		// 节点为“黑”
		rbtree_node *parent = node->parent;
		rbtree_node *brother = NULL;
		if (T->root == node){
			T->root = T->nil;
		} else if (node == parent->left) {		// 左节点 -- 右兄弟
			brother = parent->right;	// "黑"节点必有兄弟

			if (rb_node_is_red(brother)) {		// 兄弟“红色”
				left_rotate(T, parent);
				goto again;		// 转换为 黑节点 && 黑兄弟
			} else {
				RB_DEBUG("兄弟“黑色”");
				if (brother->right != T->nil) {
					RB_DEBUG("兄弟有右孩子");
					rb_set_color(brother->right, BLACK);
					left_rotate(T, parent);
				} else if (brother->left != T->nil) {	// “黑色”兄弟孩子存在，则一定是“红色”
					RB_DEBUG("兄弟无右孩子，有左孩子");
					rb_set_color(brother->left, BLACK);
					right_rotate(T, brother);
					// 转换为 兄弟有右孩子
					left_rotate(T, parent);
				} else if (rb_node_is_red(parent)) {		// 父节点“红色”
					swap(parent->color, brother->color);
				} else {								// 父节点“黑色”
					rb_set_color(brother, RED);
					node = parent;
					goto again;
				}
			}

		} else if (node == parent->right) {		// 右节点 -- 左兄弟
			brother = parent->left;

			if (rb_node_is_red(brother)) {		// 兄弟“红色”
				right_rotate(T, parent);
				goto again;		// 转换为 黑节点 && 黑兄弟
			} else {							// 兄弟“黑色”
				RB_DEBUG("兄弟“黑色”");
				if (brother->left != T->nil) {
					RB_DEBUG("兄弟有左孩子");
					rb_set_color(brother->left, BLACK);
					right_rotate(T, parent);
				} else if (brother->right != T->nil) {	// “黑色”兄弟孩子存在，则一定是“红色”
					RB_DEBUG("兄弟无左孩子，有右孩子");
					rb_set_color(brother->right, BLACK);
					left_rotate(T, brother);
					right_rotate(T, parent);
				} else if (rb_node_is_red(parent)) {		// 父节点“红色”
					swap(parent->color, brother->color);
				} else {								// 父节点“黑色”
					rb_set_color(brother, RED);
					node = parent;
					goto again;
				}
			}
		}
	}
	return;
}


int rbtree_delete(rbtree *T, const KEY_TYPE key) {
	if (T == NULL) return -1;

	rbtree_node *node = rbtree_find(T, key);
	if (node == NULL) return -1;
	RB_DEBUG_INT(node->key);

again:
	rbtree_node *child = NULL;
	rbtree_node *parent = node->parent;
	// 情形1：node 没有子节点
	if (node->left == T->nil && node->right == T->nil) {
		RB_DEBUG("情形1: node 没有子节点");
		delete_fixup(T, node);
		logic_delete(T, node, node->left);
	}
	// 情形2：node 只有一个节点
	else if (node->left != T->nil && node->right == T->nil) {
		RB_DEBUG("情形2:node 只有一个节点");
		child = node->left;
		delete_fixup(T, node);
		logic_delete(T, node, child);

	} else if (node->left == T->nil && node->right != T->nil) {
		RB_DEBUG("情形2:node 只有一个节点");
		child = node->right;
		delete_fixup(T, node);
		logic_delete(T, node, child);
	}
	// 情形3： node 有两个节点
	else /*if (node->left != T->nil && node->right != T->nil)*/ {
		RB_DEBUG("情形3: node 有两个节点");
		rbtree_node *successor = find_successor(T, node);
		// if (successor == NULL) return; 情形3 一定有后继

		swap(successor->key, node->key);
		void *tmp = successor->value;
		successor->value = node->value;
		node->value = tmp;

		node = successor;
		goto again;		// 情形3转换为情形1或2，回到上面代码
	}

	RB_DEBUG("------delete-------"); RB_DEBUG_INT(node->key);
	free(node); node = NULL;
	return 0;
}



void rbtree_print(const rbtree *T, rbtree_node *node) {
	if (T == NULL || node == NULL) return;
	if (node == T->nil) return;

	RB_DEBUG("left");
	rbtree_print(T, node->left);
	printf(" %d ," , node->key);
	RB_DEBUG("right");
	rbtree_print(T, node->right);
	RB_DEBUG("quit");
}


int rbtree_is_empty(rbtree *T) {
	return T->root == T->nil;
}


rbtree* rbtree_create() {

	rbtree *T = (rbtree*) calloc(1, sizeof(rbtree));
	if (T == NULL) {
		printf("calloc failed in %s , line: %d\n",__FUNCTION__, __LINE__);
		return NULL;
	}

	T->nil = calloc(1, sizeof(rbtree_node));
	if (T == NULL) {
		printf("calloc failed in %s\n",__FUNCTION__);
		free(T);
		return NULL;
	}
	T->nil->parent = T->nil;
	T->nil->left = T->nil->right = NULL;
	rb_set_color(T->nil, BLACK);
	T->root = T->nil;

	return T;
}


void rbtree_free(rbtree *T) {
	if (T == NULL) return;

	rbtree_node *root = NULL;
	while (!rbtree_is_empty(T)) {
		root = T->root;
		rbtree_delete(T, root->key);

	}

	free(T->nil);
	T->root = NULL;
	T->nil = NULL;

	return;
}





#if DEBUG

#define RB_NODE_SIZE_MAX  	9

int main(int argc, char** argv) {
	int i;
	rbtree *tree = rbtree_create();
	rbtree_node *rb_node_array[RB_NODE_SIZE_MAX] = {0};

	for (i = 0; i < RB_NODE_SIZE_MAX; i++) {
		rb_node_array[i] = rb_node_create(i, NULL);
		rbtree_insert(tree, rb_node_array[i]);
	}

	rbtree_print(tree, tree->root);
	rbtree_free(tree);
	tree = NULL;

	return 0;
}


#endif