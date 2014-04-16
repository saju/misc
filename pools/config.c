#include "config.h"
#include "pool.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 8096

int readline(pool_t *p, char **line, FILE *fp) {
	char *ll = NULL;
	size_t len = 0;
	size_t i = 0;

	if (feof(fp))
		return 0;

	do {
		len += 1024;
		ll = realloc(ll, len);
		
		while (i < len) {
			int c = fgetc(fp);
			if (c == EOF) {
				if (!feof(fp)) {
					pool_remember(p, ll, TAG);
					return -1;
				} else {
					ll[i++] = '\0';
					*line = ll;
					pool_remember(p, ll, TAG);
					return i;
				}
			}
			if (c == '\n') {
				ll[i++] = '\0';
				*line = ll;
				pool_remember(p, ll, TAG);
				return i;
			} else
				ll[i++] = c;
		}
	} while (len < MAX_LINE_LENGTH);
	*line = NULL;
	if (ll)
		pool_remember(p, ll, TAG);
	return -2;
}

int process_config_line(pool_t *p, config_t *conf, char *line) {
	char *value = strchr(line, '=');
	if (!value) {
		fprintf(stderr, "Bad config line format. \"%s\"\n", line);
		return 0;
	}
	value[0] = '\0';
	if (!strcmp(line, "logfile")) {
		conf->logfile = _strdup(value + 1);
		pool_remember(p, conf->logfile, "logfile name");
	} else {
		printf("Ignoring unknown config parameter \"%s\"", line);
		return 0;
	}
	return 1;
}
	
config_t *load_config(pool_t *proc, const char *conf_path) {
	char *line = NULL;
	size_t len = 0;
	int rv;
	pool_t *tmp_pool;
	config_t *conf;

	FILE *fp = fopen(conf_path, "r");
	if (!fp) {
		fprintf(stderr, "Unable to open config file %s. Error %s", conf_path, strerror(errno));
		return NULL;
	}

	tmp_pool = pool_create(proc, "temporary config pool");
	conf = pool_alloc(proc, sizeof(*conf), "config object");

	/* format is A=B\n. Lines starting with # are comments. Empty lines are allowed. WS is allowed */
	while ((rv = readline(tmp_pool, &line, fp)) > 0) {
		char *l = line;
		while (isspace(*l)) l++;
		if ((l[0] != '#') && (l[0] != '\0'))  {
			process_config_line(proc, conf, l); 
		}
	}
	if (!feof(fp)) {
		fprintf(stderr, "Error while reading config file %s. Error %s", conf_path, strerror(ferror(fp)));
		pool_destroy(tmp_pool);
		return NULL;
	}
	pool_destroy(tmp_pool);
	return conf;
}