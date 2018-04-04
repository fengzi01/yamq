#include "rbtree.h"
#include "stdio.h"
#include "stdlib.h"
#include "rbtree_util.h"

void rbtree_display(const rbtree_node_t *nil,rbtree_node_t *x) {
    if( x->left != nil )
        rbtree_display( nil,x->left );
    if( x != nil ){
        printf("%d ",x->key);
        if( x->col == RBTREE_COL_RED )
            printf("RED ");
        else
            printf("BLACK ");
        if( x->parent != nil )
            printf("%d ",x->parent->key);
        else
            printf("NULL ");
        if( x->left != nil )
            printf("%d ",x->left->key);
        else
            printf("NULL ");
        if( x->right != nil )
            printf("%d ",x->right->key);
        else
            printf("NULL ");
    }
	printf("\n");
    if( x->right != nil )
        rbtree_display(nil, x->right );
}

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
        rbtree_insert(t,i,NULL);
        printf("////////////////////////////////////////////\n");
        rbtree_dump_for_watch(t);
        printf("-------------------------------------------\n");
    }
    rbtree_dump_for_watch(t);
    rbtree_node_t *n = NULL;                   
    printf("\n\n\n////////////////////////////////////////////\n");
    printf("///////////////// DELETE //////////////////////\n");
    printf("////////////////////////////////////////////\n\n");
    for (i=1;i<10;++i) {
        n = rbtree_min(t);
        printf("////////////////////////////////////////////\n");
        printf("min = %d\n",n->key);
        rbtree_delete(t,n);
        rbtree_dump_for_watch(t);
        printf("-------------------------------------------\n");
    }


//    TEST_rbtree_travel(t);
    /*rbtree_node_t *n = NULL;                   */
    /*n = rbtree_max_node(t);                    */
    /*printf("max = %d\n",n->key);               */
    /*n = rbtree_min_node(t);                    */
    /*printf("min = %d\n",n->key);               */
    /*n = rbtree_successor_node(t,n);            */
    /*printf("successor = %d\n",n->key);         */
    /*n = rbtree_successor_node(t,n);            */
    /*printf("successor = %d\n",n->key);         */
    /*n = rbtree_successor_node(t,n);            */
    /*printf("successor = %d p = %p\n",n->key,n);*/
    /*rbtree_del_node(t,n);                      */
    /*TEST_rbtree_travel(t);                     */

    rbtree_destroy(t);
}
