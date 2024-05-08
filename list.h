#ifndef LIST_H
#define LIST_H

#define FOREACH(LIST, GENERIC_TYPE)                                                                     \
            GENERIC_TYPE it = ((LIST)->head == NULL) ? NULL : (GENERIC_TYPE)((LIST)->head)->content;    \
            for (list_node_t* curr_node = (LIST)->head;                                                 \
                 curr_node != NULL; curr_node = curr_node->next,                                        \
                                    it = (curr_node == NULL ? NULL : (GENERIC_TYPE)curr_node->content)) \

/**
 * A doubly-linked node.
 */
typedef struct ListNode {
    void *content;
    struct ListNode *next;
    struct ListNode *prev;
} list_node_t;

typedef struct List {
    list_node_t *head;
    list_node_t *tail;
    int size;
} list_t;

list_node_t *list_node_create(void *content);

list_t* list_init();
void list_add(list_t *list, void *content);
list_node_t *list_search(list_t *list, void *content, int (*matcher)(void *, void *));
int list_empty(list_t *list);
list_node_t *list_remove_head(list_t *list);
list_node_t *list_remove_tail(list_t *list);
void list_remove_node(list_t *list, list_node_t *node);
void list_free(list_t *list);

#endif //LIST_H
