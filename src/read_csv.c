#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_csv(char *path) {
    char left_spec_id[50], right_spec_id[50], label[50], first_line[33];
    int year;
    FILE *fp;
    int i = 0;
    fp = fopen(path, "r");
    //skip first row fseek or fgets
    fseek(fp, 33, SEEK_SET);
    // fgets(first_line, 33, fp);


    while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
        if (!strcmp(label, "1")) {
            printf("%s,%s,%s\n", left_spec_id, right_spec_id, label);
        }
    }
    fclose(fp);

    return (0);
}
