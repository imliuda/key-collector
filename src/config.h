#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct cfg_item cfg_item;
struct cfg_item {
    char    *name;
    void    *value;
    cfg_item *next;
};

static struct cfg_item *config = NULL;

/* parse config file to list */
void config_init(const char *file);

/* load config by name */
long int config_get_long(const char *name, long int def);
bool config_get_bool(const char *name, bool def);
char *config_get_string(const char *name, char *def);

#endif
