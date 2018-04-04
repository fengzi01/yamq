#include "rbtree.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#define N                  64//1024 * 1024
#define RBTREE_MAX_LEVEL   64
#define RBTREE_LEFT_INDEX  0
#define RBTREE_RIGHT_INDEX 1
// #define RBTREE_DEBUG
#define RBTREE_DUMP

struct node_backlog {
    /* Node backlogged */
    rbtree_node_t *node;
    /* The index next to the backtrack point, valid when >= 1 */
    int next_sub_idx;
};

static inline void
nbl_push(struct node_backlog *nbl, struct node_backlog **top, struct node_backlog **bottom)
{
    if (*top - *bottom < RBTREE_MAX_LEVEL) {
        (*(*top)++) = *nbl;
    }
}

static inline struct node_backlog *
nbl_pop(struct node_backlog **top, struct node_backlog **bottom)
{
    return *top > *bottom ? --*top : NULL;
}

static inline int
nbl_is_empty(struct node_backlog *top, struct node_backlog *bottom)
{
    return top == bottom;
}

static inline int
is_leaf(rbtree_node_t *node, rbtree_node_t *sentinel)
{
    return node->left == sentinel && node->right == sentinel;
}

static void
node_dump(rbtree_node_t *node)
{
    if (node != NULL) {
        rbtree_node_t *parent = node->parent;
        if (parent != NULL) {
            if (node == parent->left) {
                putchar('L');
            } else if (node == parent->right) {
                putchar('R');
            }
        }
        if (IS_RED(node)) {
            printf("R:%lu\n", node->key);
            //printf("R:0x%08x\n", node->key);
        } else {
            //printf("B:0x%08x\n", node->key);
            printf("B:%lu\n", node->key);
        }
    }
}

void
rbtree_dump_for_watch(rbtree_t *tree)
{
    int level = 0;
    rbtree_node_t *node = tree->root, *sentinel = tree->nil;
    struct node_backlog nbl, *p_nbl = NULL;
    struct node_backlog *top, *bottom, nbl_stack[RBTREE_MAX_LEVEL];

    top = bottom = nbl_stack;

    for (; ;) {
        if (node != sentinel) {
            /* Fetch the pop-up backlogged node's sub-id.
             * If not backlogged, fetch the first sub-id. */
            int sub_index = p_nbl != NULL ? p_nbl->next_sub_idx : RBTREE_RIGHT_INDEX;

            /* backlog should be reset since node has gone deep down */
            p_nbl = NULL;

            /* Backlog the node */
            if (is_leaf(node, sentinel) || sub_index == RBTREE_LEFT_INDEX) {
                nbl.node = sentinel;
                nbl.next_sub_idx = RBTREE_RIGHT_INDEX;
            } else {
                nbl.node = node;
                nbl.next_sub_idx = RBTREE_LEFT_INDEX;
            }
            nbl_push(&nbl, &top, &bottom);
            level++;

            /* Draw lines as long as sub_idx is the first one */
            if (sub_index == RBTREE_RIGHT_INDEX) {
                int i;
                for (i = 1; i < level; i++) {
                    if (i == level - 1) {
                        printf("%-8s", "+-------");
                    } else {
                        if (nbl_stack[i - 1].node != sentinel) {
                            printf("%-8s", "|");
                        } else {
                            printf("%-8s", " ");
                        }
                    }
                }
                node_dump(node);
            }

            /* Move down according to sub_idx */
            node = sub_index == RBTREE_LEFT_INDEX ? node->left : node->right;
        } else {
            p_nbl = nbl_pop(&top, &bottom);
            if (p_nbl == NULL) {
                /* End of traversal */
                break;
            }
            node = p_nbl->node;
            level--;
        }
    }
}
