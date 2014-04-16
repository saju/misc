#ifndef __LOG_H__
#define __LOG_H__

int log_init(pool_t *p, config_t *conf);
void log(int level, char *message, ...);

#endif
