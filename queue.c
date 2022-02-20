#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *new_head = malloc(sizeof(struct list_head));
    if (new_head) {
        INIT_LIST_HEAD(new_head);
        return new_head;
    }
    free(new_head);
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *tmp = l->next;
    while (tmp != l) {
        element_t *ptr = container_of(tmp, element_t, list);
        tmp = tmp->next;
        q_release_element(ptr);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (new &&s) {
        new->value = malloc((strlen(s) + 1) * sizeof(char));
        if (new->value) {
            strncpy(new->value, s, strlen(s));
            *(new->value + strlen(s)) = '\0';
            list_add(&new->list, head);
            return true;
        }
    }
    free(new);
    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (new &&s) {
        new->value = malloc((strlen(s) + 1) * sizeof(char));
        if (new->value) {
            strncpy(new->value, s, strlen(s));
            *(new->value + strlen(s)) = '\0';
            list_add_tail(&new->list, head);
            return true;
        }
    }
    free(new);
    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *rm_ele = container_of(head->next, element_t, list);
    list_del(head->next);

    int char_len = strlen(rm_ele->value) < (bufsize - 1) ? strlen(rm_ele->value)
                                                         : (bufsize - 1);
    if (sp) {
        sp = realloc(sp, (char_len + 1) * sizeof(char));
        strncpy(sp, rm_ele->value, char_len);
        *(sp + char_len) = '\0';
        return rm_ele;
    }
    return rm_ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *rm_ele = container_of(head->prev, element_t, list);
    list_del(head->prev);

    int char_len = strlen(rm_ele->value) < (bufsize - 1) ? strlen(rm_ele->value)
                                                         : (bufsize - 1);

    if (sp) {
        sp = realloc(sp, (char_len + 1) * sizeof(char));
        strncpy(sp, rm_ele->value, char_len);
        *(sp + char_len) = '\0';
        return rm_ele;
    }
    return rm_ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
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

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    int mid_pos =
        q_size(head) % 2 == 0 ? q_size(head) / 2 : q_size(head) / 2 + 1;

    for (int i = 0; i < mid_pos; i++)
        head = head->next;

    list_del(head);
    element_t *mid_ele = container_of(head, element_t, list);
    q_release_element(mid_ele);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_is_singular(head))
        return false;
    struct list_head *tmp = head->next;
    struct list_head *if_dup = NULL;

    while (!(tmp == head)) {
        element_t *ele_dup = container_of(tmp, element_t, list);
        element_t *ele_dup_next = container_of(tmp->next, element_t, list);

        while (!strcmp(ele_dup->value, ele_dup_next->value)) {
            if_dup = tmp;
            list_del(tmp->next);
            q_release_element(ele_dup_next);
            ele_dup_next = container_of(tmp->next, element_t, list);
            if (tmp->next == head)
                break;
        }

        tmp = tmp->next;
        if (if_dup) {
            element_t *ele_dup_a = container_of(if_dup, element_t, list);
            list_del(if_dup);
            q_release_element(ele_dup_a);
            if_dup = NULL;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_is_singular(head))
        return;

    struct list_head *forward = head->next;
    struct list_head *forward_next = forward->next;

    for (int i = 0; i < q_size(head) / 2; i++) {
        element_t *ele_first = container_of(forward, element_t, list);
        element_t *ele_second = container_of(forward_next, element_t, list);
        char *tmp = ele_first->value;
        ele_first->value = ele_second->value;
        ele_second->value = tmp;
        forward = forward_next->next;
        forward_next = forward->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_is_singular(head))
        return;

    struct list_head *forward = head->next;
    struct list_head *backward = head->prev;

    for (int i = 0; i < q_size(head) / 2; i++) {
        element_t *ele_f = container_of(forward, element_t, list);
        element_t *ele_b = container_of(backward, element_t, list);
        char *tmp = ele_f->value;
        ele_f->value = ele_b->value;
        ele_b->value = tmp;
        forward = forward->next;
        backward = backward->prev;
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    struct list_head less, greater;
    element_t *pivot, *item, *is = NULL;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&less);
    INIT_LIST_HEAD(&greater);

    pivot = list_first_entry(head, element_t, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        if (strcmp(item->value, pivot->value) < 0)
            list_move_tail(&item->list, &less);
        else
            list_move(&item->list, &greater);
    }

    q_sort(&less);
    q_sort(&greater);

    list_add(&pivot->list, head);
    list_splice(&less, head);
    list_splice_tail(&greater, head);
}
