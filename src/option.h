#ifndef __OSCLT_OPTION_H__
#define __OSCLT_OPTION_H__

#define REGISTER_OPTION(name, key, arg, flags, doc, group) \
__attribute__((constructor)) void register_option_##name(name, key, arg, flags, doc, group) { \
    register_option("name", key, "arg", flags, "doc", group); \
}

void register_option(const char *name, int key, const char *arg,
                     int flags, const char *doc, int group);

#endif
