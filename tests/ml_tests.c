#ifdef MAKEFILE
#include "../include/acutest.h"
#endif

#include "../include/ml.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
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
    ml_str_cleanup(ml, buffer);
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
    ML ml = NULL;
    char *json = "{\n"
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
    ml_create(&ml, "/home/msi/CLionProjects/k23a-project/resources/unwanted-words.txt", 1);
    JSON_ENTITY *jsonEntity = json_parse_string(json);
    dictp dict = ml_tokenize_json(ml, jsonEntity);
    char *entry = NULL;
    ulong start = 0;
    DICT_FOREACH_ENTRY(entry, dict, &start, dict->htab->buf_load) {
                TEST_CHECK(
                strcmp(entry, "micro") == 0 ||
                strcmp(entry, "sdhc") == 0 ||
                strcmp(entry, "body") == 0 ||
                strcmp(entry, "sdxc") == 0 ||
                strcmp(entry, "full") == 0 ||
                strcmp(entry, "oled") == 0 ||
                strcmp(entry, "silver") == 0 ||
                strcmp(entry, "ibis") == 0 ||
                strcmp(entry, "olympus") == 0 ||
                strcmp(entry, "cmos") == 0 ||
                strcmp(entry, "lens") == 0
        );
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
        {"string_cleanup", string_cleanup},
        {"f1_score_test",  f1_score_test},
        {"tokenize_json",  tokenize_json},
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
