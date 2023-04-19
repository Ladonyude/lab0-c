#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *ret = malloc(sizeof(struct list_head));
    if (!ret)
        return NULL;
    INIT_LIST_HEAD(ret);
    return ret;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *elt, *safe;
    list_for_each_entry_safe (elt, safe, l, list)
        q_release_element(elt);
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *elt = malloc(sizeof(element_t));
    int len = strlen(s);
    char *st = malloc(sizeof(char) * (len + 1));
    if (!st || !elt) {
        free(elt);
        free(st);
        return false;
    }
    strncpy(st, s, len);
    st[len] = '\0';
    elt->value = st;
    list_add(&elt->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *elt = malloc(sizeof(element_t));
    int len = strlen(s);
    char *st = malloc(sizeof(char) * (len + 1));
    if (!st || !elt) {
        free(elt);
        free(st);
        return false;
    }
    strncpy(st, s, len);
    st[len] = '\0';
    elt->value = st;
    list_add_tail(&elt->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = container_of(head->next, element_t, list);
    list_del_init(&node->list);
    if (sp && bufsize) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *node = container_of(head->prev, element_t, list);
    list_del_init(&node->list);
    if (sp && bufsize) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *li;
    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *slow = head->next;
    struct list_head *fast = head->next->next;
    for (; fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;
    element_t *node = list_entry(slow, element_t, list);
    list_del(&node->list);
    q_release_element(node);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    bool dup = false;
    element_t *node, *next;
    list_for_each_entry_safe (node, next, head, list) {
        if (&next->list != head && !strcmp(node->value, next->value)) {
            list_del(&node->list);
            q_release_element(node);
            dup = true;
        } else if (dup) {
            list_del(&node->list);
            q_release_element(node);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head)
        return;
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head)
            break;
        list_move(node, node->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    int count = 0;
    struct list_head *node, *next, *cut = head;
    LIST_HEAD(tmp);
    list_for_each_safe (node, next, head) {
        if (count % k == 2) {
            list_cut_position(&tmp, cut, node);
            q_reverse(&tmp);
            list_splice_init(&tmp, cut);
            cut = next->prev;
        }
        count++;
    }
}

/* merge two sorted list into one */
void q_sorted_merge_two(struct list_head *l1,
                        struct list_head *l2,
                        bool descend)
{
    if (!l1 || !l2)
        return;
    LIST_HEAD(tmp_head);
    while (!list_empty(l1) && !list_empty(l2)) {
        element_t *n1 = list_first_entry(l1, element_t, list);
        element_t *n2 = list_first_entry(l2, element_t, list);
        bool select = strcmp(n1->value, n2->value) * (1 - 2 * descend) <= 0
                          ? true
                          : false;
        element_t *nt = select ? n1 : n2;
        list_move_tail(&nt->list, &tmp_head);
    }
    if (!list_empty(l1))
        list_splice_tail(l1, &tmp_head);
    else
        list_splice_tail_init(l2, &tmp_head);
    INIT_LIST_HEAD(l1);
    list_splice(&tmp_head, l1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    LIST_HEAD(left);
    struct list_head *node = head->next, *fast = node->next;
    for (; fast != head && fast->next != head; fast = fast->next->next) {
        node = node->next;
    }
    list_cut_position(&left, head, node);
    q_sort(&left, descend);
    q_sort(head, descend);
    q_sorted_merge_two(head, &left, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *node = head->prev, *prev_node = node->prev;
    while (prev_node != head) {
        if (strcmp(list_entry(prev_node, element_t, list)->value,
                   list_entry(node, element_t, list)->value) > 0) {
            list_del_init(prev_node);
            q_release_element(list_entry(prev_node, element_t, list));
        } else {
            node = node->prev;
        }
        prev_node = node->prev;
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    struct list_head *node = head->prev, *prev_node = node->prev;
    while (prev_node != head) {
        if (strcmp(list_entry(prev_node, element_t, list)->value,
                   list_entry(node, element_t, list)->value) < 0) {
            list_del_init(prev_node);
            q_release_element(list_entry(prev_node, element_t, list));
        } else {
            node = node->prev;
        }
        prev_node = node->prev;
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *qc_first = list_first_entry(head, queue_contex_t, chain);
    queue_contex_t *qc, *qc_safe;
    int size = qc_first->size;
    list_for_each_entry_safe (qc, qc_safe, head, chain) {
        if (qc == qc_first)
            continue;
        size += qc->size;
        q_sorted_merge_two(qc_first->q, qc->q, descend);
    }
    return size;
}
