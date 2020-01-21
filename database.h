#ifndef DATABASE_H
#define DATABASE_H

#include "table.h"

typedef struct {
    char *name;
    table_t **tables;
    uint16_t table_num;
} database_t;

database_t *db_create(char *name);
void db_free(database_t *db);
size_t db_size(database_t *db);

void db_add_table(database_t *db, char *name);
table_t *db_get_table(database_t *db, char *name);
void db_store(database_t *db);

#endif