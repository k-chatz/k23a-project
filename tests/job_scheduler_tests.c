#include "../include/acutest.h"
#include "../include/job_scheduler.h"

void create_and_destroy_job_scheduler(void) {
    JobScheduler js = NULL;
    js_create(&js, 0);
    TEST_CHECK(js != NULL);
    js_destroy(&js);
    TEST_CHECK(js == NULL);
}

TEST_LIST = {
        {"create_and_destroy_job_scheduler", create_and_destroy_job_scheduler},
        {NULL, NULL}
};
