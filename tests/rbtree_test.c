#include "rbtree.h"
#include "stdio.h"
#include "stdlib.h"

void rbtree_travel(rbtree_node_t *n,rbtree_node_t *nil) {
    if (nil == n) {
        return;
    }
    rbtree_travel(n->left,nil);
    printf("%d ",n->key);
    rbtree_travel(n->right,nil);
}

void TEST_rbtree_travel(rbtree_t *tree) {
    rbtree_node_t *root = tree->root;
    rbtree_travel(root,tree->nil);
    printf("\n");
}

int main() {
    rbtree_t *t = rbtree_create(NULL);
    int i = 1;
    for (i=1;i<10;++i) {
        rbtree_insert_node(t,i,NULL);
    }

//    TEST_rbtree_travel(t);
    rbtree_node_t *n = NULL;
    n = rbtree_max_node(t);
    printf("max = %d\n",n->key);
    n = rbtree_min_node(t);
    printf("min = %d\n",n->key);
    n = rbtree_successor_node(t,n);
    printf("successor = %d\n",n->key);
    n = rbtree_successor_node(t,n);
    printf("successor = %d\n",n->key);
    n = rbtree_successor_node(t,n);
    printf("successor = %d p = %p\n",n->key,n);
    rbtree_del_node(t,n);
    TEST_rbtree_travel(t);

    rbtree_destroy(t);
}
