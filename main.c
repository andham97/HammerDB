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
    return 0;
}