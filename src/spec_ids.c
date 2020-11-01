#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/spec_ids.h"
#include "../include/spec_to_specs.h"

STS* get_spec_ids(char* path) {
    STS* spec_table = sts_new();

    struct dirent **name_list, **internals_name_list;
    int n, internals;
    char new_path[512], spec_name[1024];


    //scan external dir
    n = scandir(path, &name_list, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }

    while (n--) {
        if (!strcmp(name_list[n]->d_name, ".") || !strcmp(name_list[n]->d_name, "..")) {
            free(name_list[n]);
            continue;
        }

        snprintf(new_path, 512, "%s/%s", path, name_list[n]->d_name);

        //for each site collect specs
        internals = scandir(new_path, &internals_name_list, NULL, alphasort);
        if (internals == -1) {
            perror("scandir");
            exit(EXIT_FAILURE);
        }

        while (internals--) {
            if (!strcmp(internals_name_list[internals]->d_name, ".") ||
                !strcmp(internals_name_list[internals]->d_name, "..")) {

                free(internals_name_list[internals]);
                continue;
            }
            internals_name_list[internals]->d_name[strlen(internals_name_list[internals]->d_name) - 5] = '\0';
            snprintf(spec_name, 1024, "%s//%s", name_list[n]->d_name, internals_name_list[internals]->d_name);
            // printf("file name: %s\n", spec_name);
            sts_add(spec_table, spec_name);

            free(internals_name_list[internals]);
        }

        free(internals_name_list);
        free(name_list[n]);
    }

    free(name_list);
    return spec_table;
}
