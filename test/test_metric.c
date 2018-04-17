#include <stdio.h>
#include <stdlib.h>

#include "../src/metric.h"
#include "../src/list.h"

int main() {
    struct metric *m;
    struct json *v;
    char *ds;

    m = metric_new();
    metric_set_name(m, "test");
    metric_add_tag(m, "host", "debian");

    v = json_object();
    json_object_add(v, "user", json_integer(23));
    json_object_add(v, "sys", json_integer(36));

    metric_set_value(m, v);

    ds = metric_dumps(m);
    printf("%s\n\n", ds);
    free(ds);

    struct list *list = list_new();
    list = list_append(list, m);
    list = list_append(list, m);
    ds = metric_list_dumps(list);
    printf("%s\n\n", ds);
    free(ds);

    metric_destroy(m);


    FILE *fp = fopen("./test_metric.json", "r");
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char buf[len + 1];
    fread(buf, len, 1, fp);
    buf[len] = '\0';

    struct json *j = json_loads(buf, NULL);
    printf("dumps:\n%s\n", json_dumps(j));
    
    struct list *ms = metric_loads(buf);

    ds = metric_list_dumps(ms);
    printf("%s\n\n", ds);
    free(ds);

    metric_list_destroy(ms);
}
