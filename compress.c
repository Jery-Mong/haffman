#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

/**
 *in each item,
 *higher 24 bits store corresponding code, lower 8 bits store the code's level;
 */
int map_table[128];

#define BUF_SIZE 1024

struct element {
	int ch;
	int weight;
};
struct tree_node {
	void *data;
	struct tree_node *lchild;
	struct tree_node *rchild;
};

int cmp_element(node_t *ln1, node_t *ln2)
{
	struct tree_node *tn1 = (struct tree_node *)(ln1->data);
	struct tree_node *tn2 = (struct tree_node *)(ln2->data);
	
	return (((struct element *)(tn1->data))->weight > ((struct element *)(tn2->data))->weight) ? 1 : 0;
}

struct tree_node *list_to_tree(list_t *lst)
{
	node_t *ln1, *ln2;
	struct tree_node *tn1, *tn2, *tn3;
	struct element *em;
	
	while (lst->node_cnt > 1) {
		ln1 = list_del_tail(lst);
		ln2 = list_del_tail(lst);
		

		tn1 =(struct tree_node *)(ln1->data);
		tn2 =(struct tree_node *)(ln2->data);

		tn3 = (struct tree_node *)malloc(sizeof(struct tree_node));
		em  = (struct element *)malloc(sizeof(struct element));

		em->weight = ((struct element *)(tn1->data))->weight + ((struct element *)(tn2->data))->weight;
		em->ch = -1;

		tn3->lchild = tn1;
		tn3->rchild = tn2;
		tn3->data = (void *)em;

		ln1->data = (void *)tn3;
		
		free(ln2);
		list_sort_insert(lst, ln1, cmp_element);
	}

	ln1 = list_del_tail(lst);
	tn1 = (struct tree_node *)(ln1->data);
	
	free(ln1);
	free(lst->head);
	free(lst);
	
	return tn1;
}
void __tree_to_map_table(struct tree_node *parent, int level, int code)
{
	
	if (parent->lchild == NULL  && parent->rchild == NULL) {
		int ch = ((struct element *)(parent->data))->ch;
		map_table[ch] |= level & 0xff;
		map_table[ch] |= code & 0xffffff00;
		return;
	}		
	/* parent would nerver have only one child */
	__tree_to_map_table(parent->lchild, level + 1, code | (1 << (31 - level)));
	__tree_to_map_table(parent->rchild, level + 1, code);
	
}

void tree_to_map_table(struct tree_node *root)
{
	__tree_to_map_table(root, 0, 0);
}
list_t *weight_tb_to_list(int *weight_tb)
{
	int i;
	
	list_t *lst = (list_t *)malloc(sizeof(list_t));
	list_init(lst);
	
	for (i = 0; i < 128; i++ ) {
		/* I define 0 as the end of file */
		if (weight_tb[i] == 0 && i != 0)
			continue;
		
		node_t           *ln = (node_t *)malloc(sizeof(node_t));
		struct tree_node *tn = (struct tree_node *)malloc(sizeof(struct tree_node));
		struct element   *em = (struct element *)malloc(sizeof(struct element));


		em->weight = weight_tb[i];
		em->ch     = i;

		
		tn->data = (void *)em;
		tn->lchild = NULL;
		tn->rchild = NULL;

		ln->data = (void *)tn;
		ln->next = NULL;
		ln->prev = NULL;

		list_sort_insert(lst, ln, cmp_element);
	}
	free(weight_tb);
	return lst;
}

int *count_char_weight(char *path)
{
	FILE *fd = fopen(path, "r");

	int *tb = (int *)malloc(sizeof(int)*128);
	memset(tb, 0, sizeof(int)*128);
	
	int ch;
	while ((ch = fgetc(fd)) != EOF) {
		tb[ch]++;
	}
//	fclose(fd);
	return tb;
}

int hfm_compress(char *input, char *output)
{
	FILE *rp = fopen(input, "r");
	FILE *wp = fopen(output, "w+");
	

	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	
	int ch, i;
	int cnt = 0;
	do {
		if ((ch = fgetc(rp)) == EOF)
			ch = '\0';
		
		int code = map_table[ch] & 0xffffff00;
		int level = (char)(map_table[ch] & 0xff);
		
		for (i = 0; i < level; i++) {
			if (code & (1 << (31 - i)))
				buf[cnt / 8] |= 1 << (cnt % 8);
			cnt++;
			
			if (cnt == BUF_SIZE * 8) {
				fwrite(buf, BUF_SIZE, 1, wp);
				cnt = 0;
				memset(buf, 0, BUF_SIZE);
			}
		}
	} while(ch != '\0');
	
	fwrite(buf, cnt / 8 + 1, 1, wp);
	fclose(wp);
	
	return 0;
}

int main(int argc, char **argv)
{
	char *out_path = (char *)malloc(strlen(argv[1]));
	char *tb_path = (char *)malloc(strlen(argv[1]));
	
	strcpy(out_path, argv[1]);
	char *t = strrchr(out_path, '.');
	strcpy(t + 1, "haf");

	strcpy(tb_path, argv[1]);
	t = strrchr(tb_path, '.');
	strcpy(t + 1, "tb");
	
	int *weight_tb = count_char_weight(argv[1]);
	list_t *lst = weight_tb_to_list(weight_tb);
	
	struct tree_node *root = list_to_tree(lst);
	tree_to_map_table(root);

	

	FILE *fp = fopen(tb_path, "w+");
	fwrite(&map_table, sizeof(map_table), 1, fp);

	hfm_compress(argv[1], out_path);


	
	free(tb_path);
	free(out_path);
	return 0;
}

