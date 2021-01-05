#include "../include/acutest.h"
#include "../include/ml.h"

#ifdef MAKEFILE
#define SW_FILE "resources/unwanted-words.txt"
#else
#define SW_FILE "../resources/unwanted-words.txt"
#endif

void string_cleanup(void) {
    char stopwords[614] = "a,able,about,across,after,all,almost,also,am,among,an,and,any,are,as,at,be,because,been,but,by,can,cannot,could,dear,did,do,does,either,else,ever,every,for,from,get,got,had,has,have,he,her,hers,him,his,how,however,i,if,in,into,is,it,its,just,least,let,like,likely,may,me,might,most,must,my,neither,no,nor,not,of,off,often,on,only,or,other,our,own,rather,said,say,says,she,should,since,so,some,than,that,the,their,them,then,there,these,they,this,tis,to,too,twas,us,wants,was,we,were,what,when,where,which,while,who,whom,why,will,with,would,yet,you,your,mm,f,x,b,c,d,e,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,type,mp";
    char buffer[128] = "tHe;-. a qui,,,.ck. 255 at 43 a fox.---- j to";
    FILE *fp;
    fp = fopen(".test_stopwords", "w");
    fwrite(stopwords, sizeof(char), 614, fp);
    fclose(fp);
    ML ml = NULL;
    ml_create(&ml, ".test_stopwords", 0);
    ml_cleanup_sentence(ml, buffer);
    TEST_CHECK(strcmp(buffer, "qui ck        fox ") == 0);
    remove(".test_stopwords");
}

void f1_score_test() {
    float y[10] = {0, 1, 1, 0, 1, 1, 1, 1, 0, 0};
    float y_test[10] = {1, 1, 1, 0, 1, 0, 0, 1, 0, 0};
    float score = ml_f1_score(y, y_test, 10);
    TEST_CHECK(score == (float) 0.727272749);
}

void tokenize_json() {
    char *entry = NULL, *sentence = NULL, *token = NULL, *rest = NULL;
    StringList *json_keys = NULL;
    ulong i_state = 0;
    ML ml = NULL;

    char *json_string = "{\n"
                        "    \"<page title>\": \"Olympus OM-D E-M5 Silver w/14-42 IIR Lens\",\n"
                        "    \"image format\": \"M-43\",\n"
                        "    \"image stabilization\": \"IBIS (In Body Stabilization)\",\n"
                        "    \"lens mount\": \"Micro 4/3\",\n"
                        "    \"memory type\": \"SDHC-SDXC\",\n"
                        "    \"resolution\": \"16.1 mp\",\n"
                        "    \"sensor details\": \"CMOS\",\n"
                        "    \"video recording format\": \"Full HD\",\n"
                        "    \"viewfinder type\": \"EVF - Electronic OLED\"\n"
                        "}";
    ml_create(&ml, SW_FILE, 1);

    setp json_bow_set = set_new(10);
    dict_config(json_bow_set,
                DICT_CONF_CMP, (ht_cmp_func) strncmp,
                DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE);

    JSON_ENTITY *jsonEntity = json_parse_string(json_string);

    if (jsonEntity != NULL) {
        /* put distinct words in json_set */
        json_keys = json_get_obj_keys(jsonEntity);

        /* foreach json key*/
        LL_FOREACH(json_key, json_keys) {
            JSON_ENTITY *json_val = json_get(jsonEntity, json_key->data);
            if (json_val) {
                if (json_val->type == JSON_STRING) {
                    sentence = json_to_string(json_val);
                    ml_cleanup_sentence(ml, sentence);
                    sentence = strdup(sentence);
                    for (token = strtok_r(sentence, " ", &rest);
                         token != NULL; token = strtok_r(NULL, " ", &rest)) {
                        set_put(json_bow_set, token);
                    }
                    free(sentence);
                }
            }
        }

        dictp dict = ml_init_vocabulary_from_json_bow_set(ml, json_bow_set);

        DICT_FOREACH_ENTRY(entry, dict, &i_state, dict->htab->buf_load) {
            TEST_CHECK(
                    strcmp(entry, "om") == 0 ||
                    strcmp(entry, "stabilization") == 0 ||
                    strcmp(entry, "evf") == 0 ||
                    strcmp(entry, "sdhc") == 0 ||
                    strcmp(entry, "oled") == 0 ||
                    strcmp(entry, "silver") == 0 ||
                    strcmp(entry, "lens") == 0 ||
                    strcmp(entry, "ibis") == 0 ||
                    strcmp(entry, "electronic") == 0
            );
        }

        dict_free(json_bow_set, NULL);
    }
}

TEST_LIST = {
        {"string_cleanup", string_cleanup},
        {"f1_score_test",  f1_score_test},
        {"tokenize_json",  tokenize_json},
        {NULL, NULL}
};
