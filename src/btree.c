#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define M			6
#define DEGREES		(M/2)

typedef int KEY_TYPE;

struct btree_node
{
	KEY_TYPE *keys;		// 关键字数组
	struct btree_node **children;	// 子树数组
	unsigned int num;	// 当前关键字个数
	int leaf;		// 是否是叶子节点
};

struct btree
{
	struct btree_node *root;
};

struct btree_node* btree_create_node() {
	struct btree_node *new_node = (struct btree_node*) malloc(sizeof(struct btree_node));
	memset(new_node, 0, sizeof(struct btree_node));
	return new_node;
}


/*
T		b树
parent	分裂节点的父节点
idx		父节点的第几个
*/
void btree_split_child(struct btree *T, struct btree_node *parent, int idx) {
	unsigned int u32i, u32j;
	struct btree_node *child = parent->children[idx];
	struct btree_node *new_node = btree_create_node();
	for (u32i=DEGREES, u32j=0; u32i < child->num; u32i++, u32j++) {
		new_node->keys[u32j] = child->keys[u32i];
	}
	if (child->leaf==0) {
		new_node->leaf = 0;
		for (u32i=DEGREES, u32j=0; u32i < child->num+1; u32i++,u32j++) {
			new_node->children[u32i] = child->children[u32j];
			child->children[u32i] = NULL;
		}
	}
	parent->keys[parent->num] = child->keys[DEGREES-1];
	child->num = DEGREES-1;
	parent->children[parent->num++] = new_node;
}


void btree_insert(struct btree *T, KEY_TYPE key) {
	struct btree_node *root = T->root;
	if (root->num == M-1) {
		struct btree_node *new_node = btree_create_node();
		T->root = new_node;
		new_node->children[0] = root;
		btree_split_child(T, new_node, 0);
	}
}