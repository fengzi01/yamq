#pragma once

#ifdef __cplusplus
extern "C" {
#endif  

typedef struct _rbtree_node_t rbtree_node_t;
struct _rbtree_node_t {
    rbtree_node_t *left;
    rbtree_node_t *right;
    rbtree_node_t *parent;
    int col;
    long long key;
    void * val;
};

typedef struct _rbtree_t rbtree_t;
struct _rbtree_t {
    rbtree_node_t *root;
    rbtree_node_t *nil;
};

#define Log(format, ...)  printf(format, ##__VA_ARGS__) 

#define UNCLE_R(n) (n)->parent->parent->right
#define UNCLE_L(n) (n)->parent->parent->left
#define G_PARENT(n) (n)->parent->parent

#define RBTREE_COL_RED 1
#define RBTREE_COL_BLACK 2

#define IS_RED(n) ((n)->col==RBTREE_COL_RED)
#define IS_BLACK(n) ((n)->col==RBTREE_COL_BLACK)

#define SET_RED(n) ((n)->col=RBTREE_COL_RED)
#define SET_BLACK(n) ((n)->col=RBTREE_COL_BLACK)

#define INIT_RBTREE_NODE(nil,n, key, val) do {  \
    (n)->right = (nil);                      \
    (n)->left = (nil);                       \
    (n)->parent = (nil);                     \
    (n)->col = RBTREE_COL_RED;               \
    (n)->key = (key);                        \
} while (0)

#define root(t) (t)->root

rbtree_t *rbtree_create(void *);
void rbtree_destroy(rbtree_t *tree);

//static void _rbtree_insert_node(rbtree_t *tree,rbtree_node_t *n);
//static void left_rotate(rbtree_t *tree,rbtree_node_t *x);
//static void right_rotate(rbtree_t *tree,rbtree_node_t *x); 

rbtree_node_t * rbtree_insert(rbtree_t *tree,long long key,void *data);
void rbtree_delete(rbtree_t *tree,rbtree_node_t *n);
//void rbtree_del_node(rbtree_t *tree);

rbtree_node_t *rbtree_min(rbtree_t *tree);
rbtree_node_t *rbtree_max(rbtree_t *tree);

rbtree_node_t *rbtree_successor(rbtree_t *tree,rbtree_node_t *n);

#ifdef __cplusplus
}
#endif  
