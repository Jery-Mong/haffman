#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

/**
 * (128bits / 8) + 1 level-byte = 17 bytes
 * and there is 128 chars in ascii
 */
char *map_tb[128];

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

void __tree_to_map_tb(struct tree_node *parent, char *code)
{
	if (parent->lchild == NULL && parent->rchild == NULL)  {
		int ch = ((struct element *)(parent->data))->ch;
		map_tb[ch] = code;
		return;
	}

	int level = code[0];

	char *lcode = (char *)malloc(17);
	char *rcode = (char *)malloc(17);

	memcpy(lcode, code, 17);
	memcpy(rcode, code, 17);
	free(code);
	
	rcode[level / 8 + 1] |= 1 << (level % 8);
	rcode[0] = level + 1;
	lcode[0] = level + 1;
	
	
	__tree_to_map_tb(parent->lchild, lcode);
	__tree_to_map_tb(parent->rchild, rcode);
}

void tree_to_map_tb(struct tree_node *root)
{
	memset(map_tb, 0, 128);
	
	char *code = (char *)malloc(17);
	memset(code, 0, 17);
	
	__tree_to_map_tb(root, code);
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
		em->ch = i;
		
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

int *count_char_weight(FILE *fd)
{
	int *tb = (int *)malloc(sizeof(int)*128);
	memset(tb, 0, sizeof(int)*128);
	
	int ch;
	while ((ch = fgetc(fd)) != EOF) {
		tb[ch]++;
	}
	return tb;
}

int hfm_compress(FILE *infp, FILE *outfp)
{
	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	
	int ch, i;
	int cnt = 0;
	do {
		if ((ch = fgetc(infp)) == EOF)
			ch = '\0';
		
		char *code = map_tb[ch];
		int level = code[0];
		
		code++;		
		for (i = 0; i < level; i++) {
			if (code[i / 8] & (1 << (i % 8)))
				buf[cnt / 8] |= 1 << (cnt % 8);
			
			cnt++;
			if (cnt == BUF_SIZE * 8) {
				fwrite(buf, BUF_SIZE, 1, outfp);
				cnt = 0;
				memset(buf, 0, BUF_SIZE);
			}
		}
	} while(ch != '\0');
	
	fwrite(buf, cnt / 8 + 1, 1, outfp);
	return 0;
}
void map_tb_to_file(FILE *outfp)
{
	int i, j, k = 0;
	char buf[512];
	memset(buf, 0, 512);
	
	for (i = 0; i < 128; i++) {
		if (map_tb[i] == (void *)0) {
			k += 8;
			continue;
		}
		for (j = 0; j < map_tb[i][0] + 8; j++) {
			if (map_tb[i][j / 8] & (1 << (j % 8)))
				buf[k / 8] |= 1 << (k % 8);
			k++;
		}
	}
	fwrite(buf, 1, (k % 8) ? (k / 8 + 1) : (k / 8), outfp);
}
int main(int argc, char **argv)
{
	FILE *infp = fopen(argv[1], "r");
	if (infp == NULL) {
		perror(argv[1]);
		return -1;
	}

	char *path = (char *)malloc(strlen(argv[1]) + 4);
	strcpy(path, argv[1]);
	char *t = strrchr(path, '.');
	strcpy(t+1, "huf");

	int *weight_tb = count_char_weight(infp);
	list_t *lst = weight_tb_to_list(weight_tb);
	
	struct tree_node *root = list_to_tree(lst);
	tree_to_map_tb(root);
	
	FILE *outfp = fopen(path, "w+");
	map_tb_to_file(outfp);
	
	fseek(infp, 0, SEEK_SET);
	hfm_compress(infp, outfp);

	int i;
	for (i = 0; i < 128; i++) {
		if (map_tb[i] != 0)
			free(map_tb[i]);
	}
	fclose(infp);
	fclose(outfp);
	free(path);
	return 0;
}

