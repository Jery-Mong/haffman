#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

char *level_idx_tb[128];
char *map_tb[128];

char match_item(char *code)
{
	int idx;
	int i = 0;
	int level = code[0];
	
	while ((idx = level_idx_tb[level][i++]) != -1) {
		if (!memcmp(map_tb[idx], code, level / 8 + 2))
			return idx;	
	}	
	return -1;
}
void make_level_idx_table()
{
	int i, j;
	int level_cnt;
	char tp[128];
	
	for (i = 1; i < 128; i++) {
		for (j = 128, level_cnt = 0; j >= 0 ; --j) {
			if (map_tb[j] == (void *)0)
				continue;
			if (map_tb[j][0] == i)
				tp[level_cnt++] = j;
		}
		
		level_idx_tb[i] = (char *)malloc(level_cnt + 1); /* add one as terminated flag */
		memset(level_idx_tb[i], -1, level_cnt + 1);
		memcpy(level_idx_tb[i], tp, level_cnt);			
	}
}

int find_min_level()
{
	int i;
	int min;
	
	min = map_tb[0][0];
	for (i = 1; i < 128; i++) {
		if (map_tb[i] != (void *)0) {
			if (map_tb[i][0] < min)
				min = map_tb[i][0];
		}
	}
	return min;
}
void __hfm_extract(FILE *infp, FILE *outfp)
{
	char code_buf[BUF_SIZE];
	char char_buf[BUF_SIZE];

	int ch_cnt = 0;
	int min_level = find_min_level();
	int code_cnt;
	
	char code[17];
	memset(code, 0, 17);
	do {
		code_cnt = fread(code_buf, 1, BUF_SIZE, infp);
		
		int i;
		for (i = 0; i < code_cnt * 8; i++) {
			int level = code[0];
			if (code_buf[i / 8] & (1 << (i % 8))) 
				code[level / 8 + 1] |= 1 << (level % 8);
			
			code[0]++;
			if (code[0] < min_level)
				continue;
			
			char_buf[ch_cnt] = match_item(code);
			if (char_buf[ch_cnt] >= 0) {
				if (char_buf[ch_cnt] != '\0') {
					memset(code, 0, 17);
					ch_cnt++;
					if (ch_cnt == BUF_SIZE) {/* buffer is full */
						fwrite(char_buf, BUF_SIZE, 1, outfp);
						ch_cnt = 0;
					}
				} else /* reach the end of the file */		
					fwrite(char_buf, ch_cnt, 1, outfp);
			}
		}
	} while(code_cnt == BUF_SIZE);
}

void read_map_tb(FILE *infp)
{
	int i, j;
	int k = 0;
	char buf[512];

	fread(buf, 512, 1, infp);
	
	for (i = 0; i < 128; i++) {
		char level = 0;
		for (j = 0; j < 8; j++) {
			if (buf[k / 8] & (1 << (k % 8)))
				level |= 1 << j;
			k++;
		}
		if (level == 0)
			continue;
				
		char *code = (char *)malloc(level / 8 + 2);
		memset(code, 0, level / 8 + 2);
		
		map_tb[i] = code;
		code[0] = level;
		
		code++;
		for (j = 0; j < level; j++) {
			if (buf[k / 8] & (1 << (k % 8)))
				code[j / 8] |= 1 << (j % 8);
			k++;
		}
	}
	fseek(infp, (k % 8) ? (k / 8 + 1) : (k / 8), SEEK_SET);
}
int hfm_extract(FILE *infp, FILE *outfp)
{
	memset(map_tb, 0, 128);
	read_map_tb(infp);
	
	memset(level_idx_tb, 0, 128);
	make_level_idx_table();

	__hfm_extract(infp, outfp);

	int i;
	for (i = 0; i < 128; i++) {
		if (map_tb[i] != 0)
			free(map_tb[i]);
		if (level_idx_tb[i] != 0)
			free(level_idx_tb[i]);
	}
	return 0;
}

int main(int argc, char **argv)
{
	FILE *infp = fopen(argv[1], "r");
	if (infp == NULL) {
		perror(argv[1]);
		return -1;
	}

	char *out_path = (char *)malloc(strlen(argv[1]) + 4);
        strcpy(out_path, argv[1]);
	strcpy(strrchr(out_path, '.') + 1, "txt");

	FILE *outfp = fopen(out_path, "w+");
	hfm_extract(infp, outfp);

	fclose(infp);
	fclose(outfp);
	free(out_path);
	return 0;
}
