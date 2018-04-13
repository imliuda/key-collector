#include <stdio.h>
#include <stdlib.h>

#include "../src/json.h"

int main() {
    struct json *jo = json_object();
    struct json *ja = json_array();
    struct json *js = json_string("string value");
    struct json *ji = json_integer(123);
    struct json *jr = json_real(46.3);
    struct json *jt = json_true();
    struct json *jf = json_false();
    struct json *jn = json_null();

    struct json *jco = json_object();
    struct json *jca = json_array();

    // add some values to child object
    json_object_add(jco, "child integer", ji);
    json_object_add(jco, "child null", jn);
    json_object_add(jco, "child string", js);

    // add some values to child array
    json_array_append(jca, jt);
    json_array_append(jca, jf);
    json_array_append(jca, jr);

    // add some values to array
    json_array_append(ja, json_ref(js));
    json_array_append(ja, json_ref(jt));
    json_array_append(ja, jca);
    json_array_append(ja, json_ref(jn));
    json_array_append(ja, json_ref(ji));

    // add diffrent type values to object
    json_object_add(jo, "array", ja);
    json_object_add(jo, "string", json_ref(js));
    json_object_add(jo, "integer", json_ref(ji));
    json_object_add(jo, "real", json_ref(jr));
    json_object_add(jo, "true", json_ref(jt));
    json_object_add(jo, "false", json_ref(jf));
    json_object_add(jo, "null", json_ref(jn));

    json_object_add(jo, "child object", jco);

    /* dumps */
    char *ds;

    ds = json_dumps(jo);
    printf("%s\n\n", ds);
    free(ds);

    ds = json_dumps(ja);
    printf("%s\n\n", ds);
    free(ds);

    ds = json_dumps(js);
    printf("%s\n\n", ds);
    free(ds);

    ds = json_dumps(jt);
    printf("%s\n\n", ds);
    free(ds);

    /* destroy */
    json_destroy(jo);
}
