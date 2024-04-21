#pragma once

enum rbcolor { RB_BLACK, RB_RED };

struct rbnode {
	struct rbnode *left;
	struct rbnode *right;
	struct rbnode *parent;
	enum rbcolor color;
};

struct rbtree {
	struct rbnode *root;
	int (*compare)(void *, void *);
	void *(*get_key)(struct rbnode *);
};

void rbtree_insert(struct rbtree *, struct rbnode *);
void rbtree_delete(struct rbtree *, void *key);
struct rbnode *rbtree_min(struct rbtree *);
struct rbnode *rbtree_max(struct rbtree *);
struct rbnode *rbtree_successor(struct rbnode *);
struct rbnode *rbtree_predecessor(struct rbnode *);
struct rbnode *rbtree_search(struct rbtree *, void *key);
struct rbnode *rbtree_search_ge(struct rbtree *, void *key);
struct rbnode *rbtree_search_le(struct rbtree *, void *key);

#define container_of(ptr, type, member) \
	((type *)((char *)(ptr)-offsetof(type, member)
