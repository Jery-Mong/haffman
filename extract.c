#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

char match_item(int item, int *table)
{
	int i;

	for (i = 0; i < 128; i++) {
		if (table[i] == item)
			return i;
	}
	
	return -1;
	
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
	
	do {
		code_cnt = fread(code_buf, 1, BUF_SIZE, infp);

		int item = 0;
		int i;
		for (i = 0; i < code_cnt * 8; i++) {
			if (code_buf[i / 8] & (1 << (i % 8)))
				item |= 1 << (31 - (item & 0xff));
			
			item += 1;
			
			if ((item & 0xff) < min_level)
				continue;
			
			/* I assume match_item() always can find a corressponding char*/
			char_buf[ch_cnt] = match_item(item, table);
			
			if (char_buf[ch_cnt] != '\0') {
				if (ch_cnt == BUF_SIZE - 1) { /* buffer is full */
					fwrite(char_buf, BUF_SIZE, 1, outfp);
					ch_cnt = 0;
					memset(char_buf, 0, BUF_SIZE);
				}
				ch_cnt++;
			} else /* reach the end of the file */
				
				fwrite(char_buf, ch_cnt, 1, outfp);
		}
	} while(code_cnt == BUF_SIZE);
}
int hfm_extract(char *in_path)
{
	char *path = (char *)malloc(strlen(in_path));
        strcpy(path, in_path);
	/* create map_table's path */
	char * t = strrchr(path, '.');
	strcpy(t+1, "tb");

	int *map_table = (int *)malloc(128 * sizeof(int));
	FILE *tp = fopen(path, "r");
	fread(map_table, 128*sizeof(int), 1, tp);
	
	/* create output path */
	strcpy(t + 1, "txt");

	FILE *infp = fopen(in_path, "r");
	FILE *outfp = fopen(path, "w+");

	__hfm_extract(map_table, infp, outfp);

	fclose(infp);
	fclose(outfp);
	fclose(tp);

	free(map_table);
	
	free(path);
	
	return 0;
}
int main(int argc, char **argv)
{	
	hfm_extract(argv[1]);
	
	return 0;
}
