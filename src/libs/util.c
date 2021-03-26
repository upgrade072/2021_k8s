#include "libs.h"

int popen_cmd(const char *cmd, char *buf, int len)
{
    FILE *ptr = popen(cmd, "r");
    if (ptr == NULL) {
        fprintf(stderr, "%s() fail to run=[%s]\n", __func__, cmd);
        return -1;
    } else {
        if (fgets(buf, len, ptr) != NULL) {
            strtok(buf, "\n");
        }
        pclose(ptr);
    }
    return strlen(buf);
}   
        
void fopen_cmd(const char *path, const char *banner, const char *buffer)
{       
    FILE *fp = fopen(path, "w"); 
    if (fp == NULL) {
        fprintf(stderr, "%s() fail to write=(%s) to file=[%s]\n", __func__, banner, path);
        return;
    }
    fprintf(fp, "%s\n", banner);
    fprintf(fp, "%s", buffer);
    fflush(fp);
    fclose(fp);
}   

char *fopen_read(const char *path, char *buffer, int size)
{
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		return NULL;
	}
	char *res = fgets(buffer, size, fp);
	if (res != NULL) {
		strtok(buffer, "\n");
	}
	fclose(fp);
	return res;
}

char *fopen_read_malloc(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    char *ptr = calloc(1, size + 1);

    fseek(fp, 0, SEEK_SET);
    fread(ptr, size, 1, fp);
    strtok(ptr, "\n");

    fclose(fp);
    return ptr;
}
