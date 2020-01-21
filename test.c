#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char buf[100];
    fgets(buf, 100, stdin);
    char *table;
    char column[100];
    char value[100];
    char sep = ',';
    int num = 0;
    int off = 0;
    sscanf(buf + off, "insert into '%[^\n']' (", table);
    off += strlen(table) + 16;
    printf("TABLE: %s\n", table);
    while (sep == ',') {
        sscanf(buf + off, "%[^=]", column);
        off += strlen(column) + 1;
        printf("COLUMN: %s\n", column);
        sscanf(buf + off, "%[^),]%c", value, &sep);
        off += strlen(value) + 1;
        printf("(%c) VALUE: %s\n", sep, value);
    }
    return 0;
}