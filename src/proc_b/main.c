#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* from ARIEL */
#include <conflib.h>
#include <keepalivelib.h>

extern char *__progname;

/* from keepalive extern-library */
extern int keepaliveIndex;
extern T_keepalive *keepalive;


int main()
{
	fprintf(stderr, "--- [%s] alive! ---\n", __progname);

	if (keepalivelib_init(__progname) < 0) {
		fprintf(stderr, "%s() [%s] unregisterd to sysconfig!\n", __func__, __progname);
		return -1;
	}

	while (1) {
		fprintf(stderr, "keepalive increase(%s)!\n", __progname);
#if 1
		keepalivelib_increase();
#endif
		sleep(1);
	}

	return 0;
}
