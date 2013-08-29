#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void list_init(list_t *lst)
{
	lst->node_cnt = 0;
	lst->head = (node_t *)malloc(sizeof(node_t));
	lst->head->next = lst->head;
	lst->head->prev  = lst->head;
}


node_t* list_del_next(list_t *lst, node_t *node)
{
	node_t *tmp;

	if (node->next == node)
		return NULL;
	
	tmp = node->next;
	node->next = node->next->next;
	node->next->prev = node;
	
	lst->node_cnt--;
	return tmp;
}
node_t * list_del_after(list_t *lst, node_t *node)
{
	node_t *tmp;

	if(node->prev == node)
		return NULL;

	tmp = node->prev;
	node->prev = node->prev->prev;
	node->prev->next = node;

	lst->node_cnt--;
	return tmp;
}
int list_insert_next(list_t *lst, node_t *node1, node_t *node2)
{	
	if (node1 == NULL || node2 == NULL)
		return -1;

	node2->prev = node1;
	node2->next = node1->next;
	node2->prev->next = node2;
	node2->next->prev = node2;
	lst->node_cnt++;
	return 0;
} /* node2 -> node1 */
int list_insert_after(list_t *lst, node_t *node1, node_t *node2)
{

	if (node1 == NULL || node2 == NULL)
		return -1;
	
	node2->prev = node1->prev;
	node2->next = node1;

	node2->prev->next = node2;
	node2->next->prev = node2;
	lst->node_cnt++;
	return 0;
}
int list_del_node(list_t *lst, node_t *node)
{
	node_t *iter;
	
	if (node == NULL || lst->node_cnt == 0)
		return -1;


	for_each_node(iter, lst) {
		if (iter == node) {
			list_del_next(lst, node->prev);
			lst->node_cnt--;
			return 0;
		}
	}
	return -1;
}

int list_insert_tail(list_t *lst, node_t *node)
{
	list_insert_after(lst, lst->head, node);
	return 0;
}
int list_insert_head(list_t *lst, node_t *node)
{
	list_insert_next(lst, lst->head, node);
	return 0;
}
node_t *list_del_head(list_t *lst)
{
	return list_del_next(lst, lst->head);
}
node_t *list_del_tail(list_t *lst)
{
	return list_del_after(lst, lst->head);
}
/* we sort data from max to min */
int list_sort_insert(list_t *lst, node_t *node, int (*cmp_fn)(node_t *, node_t *))
{
	node_t *iter;
	
	if (node == NULL)
		return -1;

	if (lst->node_cnt == 0)
		list_insert_tail(lst, node);
	else if (!cmp_fn(node, list_tail(lst)))
		list_insert_tail(lst, node);
	else {
		for_each_node(iter, lst) {
			if (cmp_fn(node, iter)) {
				list_insert_after(lst, iter, node);
				break;
			}
		}
	}
	return 0;
}
