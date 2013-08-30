#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

char *level_indx_tb[24];

char match_item(int item, int *table)
{
	int idx;
	int i = 0;
	int level = item & 0xff;
	
	while ((idx = level_indx_tb[level][i++]) != -1) {
		if (item == table[idx])
			return idx;
	}
	return -1;
}
void make_level_index_table(int *map_table)
{
	int i, j;
	int level_cnt;
	char tp[128];

	
	for (i = 1; i < 24; i++) {
		for (j = 128, level_cnt = 0; j >= 0 ; --j) {
			if ((map_table[j] & 0xff) == i)
				tp[level_cnt++] = j;
		}

		level_indx_tb[i] = (char *)malloc(level_cnt + 1);
		memset(level_indx_tb[i], -1, level_cnt + 1);
		memcpy(level_indx_tb[i], tp, level_cnt);
	}
		
}

int find_min_level(int *table)
{
	int i;
	int min;
	
	min = table[0] & 0xff;
	for (i = 1; i < 128; i++) {
		if ((table[i] & 0xff) < min && (table[i] & 0xff) != 0)
			min = table[i] & 0xff; 
	}
	return min;
}
void __hfm_extract(int *table, FILE *infp, FILE *outfp)
{
	char code_buf[BUF_SIZE];
	char char_buf[BUF_SIZE];

	int ch_cnt = 0;
	int min_level = find_min_level(table);
	int code_cnt;
	int item = 0;
	
	do {
		code_cnt = fread(code_buf, 1, BUF_SIZE, infp);

		int i;
		for (i = 0; i < code_cnt * 8; i++) {
			if (code_buf[i / 8] & (1 << (i % 8)))
				item |= 1 << (31 - (item & 0xff));
			
			item += 1;
			
			if ((item & 0xff) < min_level)
				continue;
			
			char_buf[ch_cnt] = match_item(item, table);
			if (char_buf[ch_cnt] >= 0) {
				if (char_buf[ch_cnt] != '\0') {
					item = 0;
					ch_cnt++;
					
					if (ch_cnt == BUF_SIZE) { /* buffer is full */
						fwrite(char_buf, BUF_SIZE, 1, outfp);
						ch_cnt = 0;
					}
				} else /* reach the end of the file */		
					fwrite(char_buf, ch_cnt, 1, outfp);
			}
		}
	} while(code_cnt == BUF_SIZE);
}
int hfm_extract(FILE *infp, FILE *outfp)
{
	
	int *map_table = (int *)malloc(128 * sizeof(int));
	fread(map_table, 128*sizeof(int), 1, infp);

	make_level_index_table(map_table);
	
	/* create output path */

	__hfm_extract(map_table, infp, outfp);
	
	free(map_table);
	
	return 0;
}

int main(int argc, char **argv)
{
	FILE *infp = fopen(argv[1], "r");
	if (infp == NULL) {
		perror(argv[1]);
		return -1;
	}

	char *out_path = (char *)malloc(strlen(argv[1]));
        strcpy(out_path, argv[1]);
	strcpy(strrchr(out_path, '.') + 1, "txt");

	FILE *outfp = fopen(out_path, "w+");
	
	hfm_extract(infp, outfp);

	fclose(infp);
	fclose(outfp);
	free(out_path);
	return 0;
}
