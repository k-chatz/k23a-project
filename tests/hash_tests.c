#include <fcntl.h>
#include <unistd.h>
#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/acutest.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void put_get_string(void) {
    hashp hash = htab_new(djb2_str, 10, 10, 5000);
    htab_put(hash, "foo", "foo");
    char *foo = htab_get(hash, "foo");
    TEST_CHECK((strcmp("foo", foo) == 0));
}

void rehash(void) {
    hashp hash = htab_new(djb2_str, 10, 10, 50);
    hash->keycpy = (ht_key_cpy_func)strncpy;
    hash->cmp = (ht_cmp_func)strncmp;
    htab_put(hash, "foo", "foo");

    hashp bigger = htab_new(djb2_str, 100, 100, 5000);
    bigger->keycpy = (ht_key_cpy_func)strncpy;
    bigger->cmp = (ht_cmp_func)strncmp;
    htab_rehash(hash, bigger);
    
    char *foo = htab_get(bigger, "foo");
    TEST_CHECK((strcmp(foo, "foo") == 0));

    free(hash);
    free(bigger);
}

void put_get_int_pointer() {
    int *a = NULL, **b = NULL, c = 10;
    a = &c;
    hashp json_ht = htab_new(djb2_str, 128, sizeof(int *), 10);
    TEST_ASSERT(json_ht != NULL);
    TEST_CHECK(htab_put(json_ht, "key", &a));
    b = (int **) htab_get(json_ht, "key");
    TEST_ASSERT(b != NULL);
    TEST_CHECK(a == *b);
}

void put_get_json_entity(void) {
    JSON_ENTITY *json_a = NULL, **json_b = NULL;
    hashp json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), 10);
    TEST_ASSERT(json_ht != NULL);
    json_a = json_to_entity("{\n"
                            "  \"<page title>\": \"Olympus OM-D E-M10 Black Digital Camera (16.1 MP, SD/SDHC/SDXC Card Slot) Price Comparison at Buy.net\",\n"
                            "  \"camera body only\": \"Body Only\",\n"
                            "  \"camera type\": \"Mirrorless Interchangeable Lens Camera\",\n"
                            "  \"depth\": \"1.8 in\",\n"
                            "  \"effective megapixels\": \"16100000 pixels\",\n"
                            "  \"environmental protection\": \"Water Resistant\"\n"
                            "}");
    TEST_ASSERT(json_a != NULL);
    TEST_CHECK(htab_put(json_ht, "key", &json_a));
    json_b = htab_get(json_ht, "key");
    TEST_ASSERT(json_b != NULL);
    TEST_CHECK(json_a == *json_b);
}

/* NOT A TEST */
void inc(void *val){
    (*(int*)val)++;
}

void update(void) {
    hashp ht = htab_new(djb2, sizeof(int), sizeof(int), 10);
    int key = 5, value = 0;
    for(int i = 0; i < 5; i++){
	htab_update(ht, &key, &value, inc);
	value = *(int*)htab_get(ht, &key);
	TEST_CHECK(i == value);
    }
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
    {"put_get_string",      put_get_string},
    {"put_get_int_pointer", put_get_int_pointer},
    {"put_get_json_entity", put_get_json_entity},
    {"rehashing", rehash},
    {"update", update},
    {NULL, NULL}
};

#ifndef ACUTEST_H

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; test_list_[i].name != NULL; i++) {
        test_list_[i].func();
    }
    return 0;
}

#endif
