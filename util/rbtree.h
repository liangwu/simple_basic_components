#ifndef __RBTREE_H__
#define __RBTREE_H__


typedef int KEY_TYPE;	// Key type


typedef struct _rb_node{
	unsigned char color;
	struct _rb_node *left;
	struct _rb_node *right;
	struct _rb_node *parent;
	KEY_TYPE key;
	void *value;
} rbtree_node;

typedef struct _rbtree {
	struct _rb_node *root;
	struct _rb_node *nil;	// 通用结点
} rbtree;


#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

extern KEY_TYPE key_compare(const KEY_TYPE *x, const KEY_TYPE *y);
extern rbtree_node* rbtree_find(rbtree *T, const KEY_TYPE key);
extern void rbtree_insert(rbtree *T, rbtree_node *new_node);
extern int rbtree_delete(rbtree *T, const KEY_TYPE key);
extern rbtree* rbtree_create();
extern void rbtree_free(rbtree *T);


#ifdef __cplusplus
}
#endif // !__cplusplus


#endif // !__RBTREE_H__


