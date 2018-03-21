#include "rbtree.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

/**
 *  left_rotate
 *            .-.                 
 *            (11 )                
 *            /`-'\                
 *      /----/     \---\           
 *  .-.v                v.-.       
 * ( 9 )                (18 )      
 *  `-'                  /-'       
 *                   /--/          
 *                 .v.             
 *               /(14 \            
 *           /--/  `-' \---\       
 *         .v.              v-.    
 *        (12 )            (17 )   
 *         `-'              `-'    
 *                                 
 *            |    |               
 *            |    |               
 *            v    v               
 *                    .-.          
 *                   (18 )         
 *                   /`-'          
 *                /-/              
 *            .-.v                 
 *           (11 )                 
 *           /`-'\                 
 *     /----/     \--\             
 * .-.v               v-.          
 *( 9 )             /(14 \         
 * `-'          /--/  `-' \---\    
 *            .v.              v-. 
 *           (12 )            (17 )
 *            `-'              `-' 
 **/

static void left_rotate(rbtree_t *tree,rbtree_node_t *x) {
    if (x->right == tree->nil)  return;
    rbtree_node_t *y = x->right;

    // 1. 将y的左子树放到x的右子树
    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    // 2. 连接与P节点的双向连接
    y->parent = x->parent;
    if (y->parent == tree->nil) {
        tree->root = y;
    } else {
        if (x == y->parent->left) {
            y->parent->left = y;
        } else {
            y->parent->right = y;
        }
    }

    // 3. 连接x与y的双向连接
    x->parent = y;
    y->left = x;
}

/**
 *                    .-.          
 *                   (18 )         
 *                   /`-'          
 *                /-/              
 *            .-.v                 
 *           (11 )                 
 *           /`-'\                 
 *     /----/     \--\             
 * .-.v               v-.          
 *( 9 )             /(14 \         
 * `-'          /--/  `-' \---\    
 *            .v.              v-. 
 *           (12 )            (17 )
 *            `-'              `-' 
 *            
 *            |    |               
 *            |    |               
 *            v    v               
 *
 *            .-.                 
 *            (11 )                
 *            /`-'\                
 *      /----/     \---\           
 *  .-.v                v.-.       
 * ( 9 )                (18 )      
 *  `-'                  /-'       
 *                   /--/          
 *                 .v.             
 *               /(14 \            
 *           /--/  `-' \---\       
 *         .v.              v-.    
 *        (12 )            (17 )   
 *         `-'              `-'    
 */
static void right_rotate(rbtree_t *tree,rbtree_node_t *x) {
    if (x->left == tree->nil) return; 
    rbtree_node_t *y = x->left;

    x->left = y->right;
    if (x->left != tree->nil) {
        x->left->parent = x;
    }

    y->parent = x->parent;
    if (y->parent == tree->nil) {
        tree->root = y;
    } else {
        if (y->parent->left == x) {
            y = y->parent->left;
        } else {
            y = y->parent->right;
        }
    }

    x->parent = y;
    y->right = x;
}

static void rbtree_insert_fixup(rbtree_t *tree,rbtree_node_t *node) {
    rbtree_node_t *n = node;
    while (/*n != tree->root && */n->parent->col == RBTREE_COL_RED) {
        if (n->parent == G_PARENT(n)->left) {
            // 父亲节点是祖父节点的左子树
            rbtree_node_t *uncle = UNCLE_R(n);
            if (uncle->col == RBTREE_COL_RED) {
                n->parent->col = RBTREE_COL_BLACK;
                uncle->col = RBTREE_COL_BLACK;
                G_PARENT(n)->col = RBTREE_COL_RED;
                n = G_PARENT(n);
            } else {
                if(n == n->parent->right) {
                    n = n->parent;
                    left_rotate(tree,n);
                }
                n->parent->col = RBTREE_COL_BLACK;
                G_PARENT(n)->col = RBTREE_COL_RED;
                right_rotate(tree,G_PARENT(n));
            }
        } else {
            // 对称
            rbtree_node_t *uncle = UNCLE_L(n);
            if (uncle->col == RBTREE_COL_RED) {
                n->parent->col = RBTREE_COL_BLACK;
                uncle->col = RBTREE_COL_BLACK;
                G_PARENT(n)->col = RBTREE_COL_RED;
                n = G_PARENT(n);
            } else {
                if(n == n->parent->left) {
                    n = n->parent;
                    right_rotate(tree,n);
                }
                n->parent->col = RBTREE_COL_BLACK;
                G_PARENT(n)->col = RBTREE_COL_RED;
                left_rotate(tree,G_PARENT(n));
            }
        }
    }
    tree->root->col = RBTREE_COL_BLACK;
}

static void _rbtree_insert_node(rbtree_t *tree,rbtree_node_t *n) {
    rbtree_node_t *x = tree->root;
    rbtree_node_t *p = x;

    while (x != tree->nil) {
        p = x;
        if (n->key < x->key) {
            x = x->left;
        } else if (n->key > x->key) {
            x = x->right;
        } else  {
           // printf("key is exist!\n");
            return;
        }
    }
    n->parent = p;
    if (p == tree->nil) {
        // empty
        tree->root = n;
        //printf("insert node is root\n");
        //        return;
    } else {
        if (n->key < p->key) {
            p->left = n;
        } else {
            p->right = n;
        }
    }
    n->left = n->right = tree->nil;
    n->col = RBTREE_COL_RED;
    rbtree_insert_fixup(tree,n);
}

rbtree_node_t *rbtree_insert_node(rbtree_t *t,long long key,void *data) {
    rbtree_node_t *n = (rbtree_node_t *)malloc(sizeof(rbtree_node_t));
    n->key = key;
    n->val = data;

    _rbtree_insert_node(t,n);
    return n;
}

rbtree_t* rbtree_create(void *p) {
    (void)p;
    rbtree_t *tree = NULL;
    tree = (rbtree_t *)malloc(sizeof(rbtree_t));
    assert(tree);

    tree->nil = (rbtree_node_t *)malloc(sizeof(rbtree_node_t));
    tree->nil->left = tree->nil->right = tree->nil->parent = NULL;
    tree->nil->col = RBTREE_COL_BLACK;

    tree->root = tree->nil;
    return tree;
} 

void rbtree_destroy(rbtree_t *t) {
}

rbtree_node_t * rbtree_min_node(rbtree_t *t) {
    rbtree_node_t *min = t->root;
    rbtree_node_t *x = t->root;
    int i = 0;
    while (x != t->nil) {
        min = x;
        x = x->left;
        ++i;
    }
    //printf("min len = %d key = %lu\n",i,min->key);
    return min;
}

rbtree_node_t *rbtree_max_node(rbtree_t *t) {
    rbtree_node_t *max = t->root;
    rbtree_node_t *x = t->root;
    int i = 0;
    while (x != t->nil) {
        max = x;
        x = x->right;
        ++i;
    }
    //printf("max len = %d\n",i);

    return max;
}

rbtree_node_t *rbtree_successor_node(rbtree_t *t,rbtree_node_t *n) {
    if (n == t->nil) return NULL;
    rbtree_node_t *x = NULL;
    rbtree_node_t *p = NULL; 

    if (n->right != t->nil) {
        // 右子树的最小值
        x = n->right;
        p = x; 
        while ( x != t->nil) {
            p = x;
            x = x->left;
        }
        return p;
    } else {
        // 第一个非右子树的父节点
        p = n->parent; 
        x = n;
        while (p != t->root && p->right == x) {
            x = p;
            p = p->parent;
        }
        return p;
    }
}

static void rbtree_del_fixup(rbtree_t *t,rbtree_node_t *x) {
    while (x != t->root && x->col == RBTREE_COL_BLACK) {
        rbtree_node_t *w = NULL;
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->col == RBTREE_COL_RED) { // case1
                w->col = RBTREE_COL_BLACK;
                x->parent->col = RBTREE_COL_RED;
                left_rotate(t,x->parent);
                w = x->parent->right; // w 必然为黑色
            }
            // FIXME 有空节点咋办
            if (w->left->col == RBTREE_COL_BLACK && w->right->col == RBTREE_COL_BLACK) { // case2
                w->col = RBTREE_COL_RED;
                x = x->parent;
            } else {
                if (w->right->col == RBTREE_COL_BLACK) { // case3
                    w->left->col = RBTREE_COL_BLACK;
                    w->col = RBTREE_COL_RED;
                    right_rotate(t,w);
                    w = x->parent->right;
                }
                w->col = x->parent->col; // case4
                x->parent->col = RBTREE_COL_BLACK;
                w->right->col = RBTREE_COL_BLACK;
                left_rotate(t,x->parent);
                x = t->root;  // why?
            }
        } else {
            w = x->parent->left;
            if (w->col == RBTREE_COL_RED) { // case1
                w->col = RBTREE_COL_BLACK;
                x->parent->col = RBTREE_COL_RED;
                right_rotate(t,x->parent);
                w = x->parent->left; // w 必然为黑色
            }
            // FIXME 有空节点咋办
            if (w->left->col == RBTREE_COL_BLACK && w->right->col == RBTREE_COL_BLACK) { // case2
                w->col = RBTREE_COL_RED;
                x = x->parent;
            } else {
                if (w->left->col == RBTREE_COL_BLACK) { // case3
                    w->right->col = RBTREE_COL_BLACK;
                    w->col = RBTREE_COL_RED;
                    left_rotate(t,w);
                    w = x->parent->left;
                }
                w->col = x->parent->col; // case4
                x->parent->col = RBTREE_COL_BLACK;
                w->right->col = RBTREE_COL_BLACK;
                right_rotate(t,x->parent);
                x = t->root;  // why?
            }
        }
    }
    x->col = RBTREE_COL_BLACK;
}

void rbtree_del_node(rbtree_t *t,rbtree_node_t *z) {
    rbtree_node_t *y = NULL, *x = NULL;
    if (z->left != t->nil && z->right != t->nil) {
        // 找到后继节点
        y = z->right;
        while ( y->left != t->nil) {
            y = y->left;
        }
        // 用y代替z删除
        void *val = z->val;
        long long key = z->key;
        z->val = y->val;
        z->key = y->key;
        y->val = val;
        y->key = key;
    } else {
        y = z;
    }

    // y 才是要删除的节点！！！
    if (y->left == t->nil) {
        x = y->left;
    } else {
        // FIXME x可能为nil
        x = y->right;
    }
    // 用x来代替y
    // FIXME x = nil时？
    x->parent = y->parent;
    if (y->parent == t->nil) {
        t->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    /*
    if (y != z) {
        z->val = y->val;
    }
    */
    if (y->col == RBTREE_COL_BLACK) {
        rbtree_del_fixup(t,x);
    }
    fprintf(stderr,"del node key = %lu\n",y->key);
    free(y);
}
