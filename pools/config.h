#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "pool.h"

#define DEBUG 1
#define INFO 2
#define WARN 3
#define ERR 4
#define CRIT 5

typedef struct {
	char *logfile;
}config_t;

config_t *load_config(pool_t *pool, const char *conf_path);

#endif
