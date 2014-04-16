#include "config.h"
#include "pool.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

FILE *logf = NULL;
int configured_level = INFO;

void close_logfile(void *data) {
	FILE *fp = (FILE *)data;
	fclose(fp);
	printf("closed logfile");
}

int log_init(pool_t *p, config_t *conf) {
	logf = fopen(conf->logfile, "a+");
	if (!logf) {
		fprintf(stderr, "Unable to open logfile \"%s\". Error: %s\n", conf->logfile, strerror(errno));
		return 0;
	}
	setvbuf(logf, NULL, _IOLBF);
	register_cleanup(p, close_logfile, logf, "log cleanup handler");
	return 1;
}

void log(int level, char *msg, ...) {
	va_list args;
	
	if (level < configured_level)
		return;
	 
	va_start(args, msg);
	//fprintf(logf, "[INFO] [tstamp] "");
	vfprintf(logf, msg, args);
	fprintf("\n");
	va_end(args);
}
