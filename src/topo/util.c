#include "topo.h"

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
