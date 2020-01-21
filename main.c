#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "table.h"
#include "database.h"

int main() {
    char *db_name = "Main";
    char *table_name = "Person";
    char *shift_table_name = "Shift Table";
    database_t *db = db_create(db_name);
    db_add_table(db, table_name);
    table_t *table = db_get_table(db, table_name);

    table_add_column(table, "Id", NUMBER, 1, INCREMENTAL);
    table_add_column(table, "Name", STRING, '\0', NONE);
    table_add_column(table, "Deleted", BOOL, 1, NONE);
    table_add_column(table, "Employee Number", NUMBER, 100, INCREMENTAL);

    table_insert_data(table, "Name,Employee Number", "Alice", 200);
    table_insert_data(table, "Name", "Bob");
    table_insert_data(table, "Name", "Claus");
    table_insert_data(table, "Name", "Simon");
    table_insert_data_structured(table, 2, 1, "Brook");

    table_print(table);

    db_add_table(db, shift_table_name);
    table = db_get_table(db, shift_table_name);

    table_add_column(table, "Id", NUMBER, 1, INCREMENTAL);
    table_add_column(table, "PersonId", NUMBER, 0, NONE);
    table_add_column(table, "Shift Start", NUMBER, 0, NONE);
    table_add_column(table, "Shift End", NUMBER, 0, NONE);

    table_insert_data_structured(table, 4, 0, 1, 840, 1080);
    table_insert_data_structured(table, 4, 0, 2, 1080, 60);

    table_print(table);

    db_store(db);
    db_free(db);

    /*char *table_name = "Person";
    table_t *table = table_create(table_name);
    table_add_column(table, "Id", NUMBER, 1, INCREMENTAL);
    table_add_column(table, "Name", STRING, '\0', NOT_NULL);
    table_add_column(table, "Deleted", BOOL, 1, NONE);
    table_insert_data(table, 2, 1, "Andreas");
    table_insert_data(table, 2, 1, "Odin");
    table_insert_data(table, 2, 1, "Ole Kristian");
    table_insert_data(table, 2, 1, "Emilie");
    table_print(table);

    char table_file[strlen(table->name) + 5];
    sprintf(table_file, "%s%s%s", "./", table->name, ".db");

    int table_fp = open("./tables.db", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    int table_data_fp = open(table_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    size_t size = table_size(table);
    size_t data_size = 0;
    void *table_data = malloc(size);
    table_serialize(table, table_data);

    for (int i = 0; i < table->row_num; i++) {
        write(table_data_fp, table->row_data[i], table->row_size[i]);
        data_size += table->row_size[i];
    }

    write(table_fp, table_data, size);

    ftruncate(table_fp, size);
    ftruncate(table_data_fp, data_size);
    table_free(table);
    free(table_data);
    
    off_t file_size = lseek(table_fp, 0, SEEK_END);
    lseek(table_fp, 0, SEEK_SET);
    void *table_file = malloc(file_size);
    read(table_fp, table_file, file_size);
    table = table_deserialize(table_file);
    table_print(table);
    table_free(table);
    free(table_file);*/
    return 0;
}