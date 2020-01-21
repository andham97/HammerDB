#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef enum {
    NUMBER = 1,
    STRING,
    BOOL
} column_type;

typedef enum {
    NONE,
    INCREMENTAL
} column_function;

typedef enum {
    SUCCESS,
    INVALID_VALUE,
    UNKNOWN_FUNCTION
} insert_data_error;

typedef struct {
    char *name;
    column_type type;
    uint8_t default_value;
    column_function function;
    void *meta;
    uint16_t meta_length;
} column_t;

typedef struct {
    char *name;
    uint16_t column_num;
    column_t **structure;
    void **row_data;
    uint32_t row_num;
    uint32_t *row_size;
} table_t;

table_t *table_create(char *name);
void table_free(table_t *table);
void table_print(table_t *table);
size_t table_size(table_t *table);
size_t table_data_size(table_t *table);

void table_add_column(table_t *table, char *name, column_type type, uint8_t default_value, column_function function);

insert_data_error table_insert_data(table_t *table, const char *format, ...);
insert_data_error table_insert_data_structured(table_t *table, int argc, ...);

void table_serialize(table_t *table, void *destination);
void table_serialize_off(table_t *table, void *destination, size_t offset);
table_t *table_deserialize(void *source);
table_t *table_deserialize_off(void *source, size_t offset);

#endif