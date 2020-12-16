#include "../include/acutest.h"
#include "../include/ml.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

void string_cleanup(void) {
    char stopwords[614] = "a,able,about,across,after,all,almost,also,am,among,an,and,any,are,as,at,be,because,been,but,by,can,cannot,could,dear,did,do,does,either,else,ever,every,for,from,get,got,had,has,have,he,her,hers,him,his,how,however,i,if,in,into,is,it,its,just,least,let,like,likely,may,me,might,most,must,my,neither,no,nor,not,of,off,often,on,only,or,other,our,own,rather,said,say,says,she,should,since,so,some,than,that,the,their,them,then,there,these,they,this,tis,to,too,twas,us,wants,was,we,were,what,when,where,which,while,who,whom,why,will,with,would,yet,you,your,mm,f,x,b,c,d,e,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,type,mp";
    char buffer[128] = "tHe;-. a qui,,,.ck. 255 at 43 a fox.---- j to";
    FILE * fp;
    fp = fopen(".test_stopwords", "w");
    fwrite(stopwords, sizeof(char), 614, fp);
    fclose(fp);
    ML ml = NULL;
    ml_create(&ml, ".test_stopwords", 0);
    ml_str_cleanup(ml, buffer);
    TEST_CHECK(strcmp(buffer, "qui ck        fox ") == 0);
    remove(".test_stopwords");
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"string cleanup", string_cleanup},
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
