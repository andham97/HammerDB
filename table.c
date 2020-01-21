#include "table.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void table_add_column(table_t *table, char *name, column_type type, uint8_t default_value, column_function function) {
    if (table->column_num == 0) {
        table->structure = malloc(sizeof(column_t *));
        table->column_num++;
    }
    else {
        table->column_num++;
        table->structure = realloc(table->structure, sizeof(column_t *) * table->column_num);
    }
    int pos = table->column_num - 1;
    table->structure[pos] = malloc(sizeof(column_t));
    table->structure[pos]->name = name;
    table->structure[pos]->type = type;
    table->structure[pos]->default_value = default_value;
    table->structure[pos]->function = function;
    table->structure[pos]->meta = NULL;
    table->structure[pos]->meta_length = 0;
    if (function == INCREMENTAL && type == NUMBER) {
        table->structure[pos]->meta = malloc(sizeof(int));
        int num = (int)default_value;
        memcpy(table->structure[pos]->meta, &num, sizeof(int));
        table->structure[pos]->meta_length = sizeof(int);
    }
}

insert_data_error value_function(column_t *column, void *value) {
    if (column->function == INCREMENTAL) {
        if (column->type == NUMBER) {
            int meta;
            memcpy(&meta, column->meta, sizeof(int));
            memcpy(value, &meta, sizeof(int));
            meta++;
            memcpy(column->meta, &meta, sizeof(int));
            return SUCCESS;
        }
    }
    else if (column->function == NONE) {
        return SUCCESS;
    }
    return UNKNOWN_FUNCTION;
}

insert_data_error table_insert_data_raw(table_t *table, const char *format, va_list args) {
    insert_data_error err = SUCCESS;
    
    size_t off = 0;
    size_t format_length = strlen(format);
    char **columns = malloc(table->column_num * sizeof(char *));
    char c = ',';
    int count = 0;

    char *column_name = malloc(strlen(format) + 1);
    while (off < format_length && c == ',') {
        int num_args = sscanf(format + off, "%[^\n,]%c", column_name, &c);
        if (strlen(column_name) > 0) {
            columns[count] = malloc(strlen(column_name) + 1);
            printf("%s\n", column_name);
            memcpy(columns[count], column_name, strlen(column_name) + 1);
            off += strlen(column_name);
            count++;
        }
        if (num_args == 2) {
            off += 1;
        }
        else {
            break;
        }
    }
    free(column_name);

    int cur_loc = 0;
    for (int i = 0; i < table->column_num; i++) {
        int found = -1;
        for (int j = 0; j < count; j++) {
            if (strcmp(table->structure[i]->name, columns[j]) == 0 && strlen(table->structure[i]->name) - strlen(columns[j]) == 0) {
                found = j;
                break;
            }
        }
        if (found != -1 && cur_loc != found) {
            char *tmp = columns[cur_loc];
            columns[cur_loc] = columns[found];
            columns[found] = tmp;
            cur_loc++;
        }
        else if (cur_loc == found) {
            cur_loc++;
        }
    }

    cur_loc = 0;
    size_t row_size = 0;
    void *data = NULL;
    for (int i = 0; i < table->column_num && err == 0; i++) {
        column_type type = table->structure[i]->type;
        int provided = 0;
        if (columns[cur_loc] != NULL && strcmp(table->structure[i]->name, columns[cur_loc]) == 0 && strlen(table->structure[i]->name) - strlen(columns[cur_loc]) == 0) {
            provided = 1;
            cur_loc++;
        }
        if (type == NUMBER) {
            int num;
            if (provided > 0) {
                num = va_arg(args, int);
            }
            else {
                num = (int)table->structure[i]->default_value;
            }
            err = value_function(table->structure[i], &num);
            if (err != SUCCESS)
                break;
            if (data == NULL) {
                data = malloc(sizeof(int));
            }
            else {
                data = realloc(data, row_size + sizeof(int));
            }
            memcpy(data + row_size, &num, sizeof(int));
            row_size += sizeof(int);
        }
        else if (type == STRING) {
            char *string;
            if (provided > 0) {
                string = va_arg(args, char *);
            }
            else {
                string = malloc(1);
                string[0] = (char)table->structure[i]->default_value;
            }
            err = value_function(table->structure[i], string);
            if (err != SUCCESS)
                break;
            uint16_t string_length = strlen(string) + 1;
            if (data == NULL) {
                data = malloc(sizeof(uint16_t));
            }
            else {
                data = realloc(data, row_size + sizeof(uint16_t));
            }
            memcpy(data + row_size, &string_length, sizeof(uint16_t));
            row_size += sizeof(uint16_t);
            data = realloc(data, row_size + string_length);
            memcpy(data + row_size, string, string_length);
            row_size += string_length;
        }
        else if (type == BOOL) {
            uint8_t byte;
            if (provided > 0) {
                byte = (uint8_t)va_arg(args, int);
            }
            else {
                byte = table->structure[i]->default_value;
            }
            err = value_function(table->structure[i], &byte);
            if (err != SUCCESS)
                break;
            if (data == NULL) {
                data = malloc(sizeof(uint8_t));
            }
            else {
                data = realloc(data, row_size + sizeof(uint8_t));
            }
            memcpy(data + row_size, &byte, sizeof(uint8_t));
            row_size += sizeof(uint8_t);
        }
    }
    if (err == SUCCESS) {
        if (table->row_num == 0) {
            table->row_data = malloc(sizeof(void *));
            table->row_size = malloc(sizeof(uint32_t));
            table->row_num++;
        }
        else {
            table->row_num++;
            table->row_data = realloc(table->row_data, sizeof(void *) * table->row_num);
            table->row_size = realloc(table->row_size, sizeof(uint32_t) * table->row_num);
        }
        table->row_data[table->row_num - 1] = data;
        table->row_size[table->row_num - 1] = row_size;
    }

    for (int i = 0; i < count; i++) {
        if (columns[i] != NULL) {
            free(columns[i]);
        }
    }
    free(columns);
    return err;
}

insert_data_error table_insert_data(table_t *table, const char *format, ...) {
    va_list args;
    va_start(args, format);
    insert_data_error err = table_insert_data_raw(table, format, args);
    va_end(args);
    return err;
}

insert_data_error table_insert_data_structured(table_t *table, int argc, ...) {
    va_list args;
    va_start(args, argc);

    size_t format_length = 1;
    size_t off = 0;
    char *comma = ",";
    char *format = malloc(format_length);

    for (int i = 0; i < table->column_num && i < argc; i++) {
        format_length += strlen(table->structure[i]->name);
        if (i < argc - 1) {
            format_length++;
        }
        format = realloc(format, format_length);
        memcpy(format + off, table->structure[i]->name, strlen(table->structure[i]->name) + 1);
        off += strlen(table->structure[i]->name);
        if (i < argc - 1) {
            memcpy(format + off, comma, 2);
            off += 1;
        }
    }

    insert_data_error err = table_insert_data_raw(table, format, args);

    va_end(args);
    return err;
}

size_t table_size(table_t *table) {
    size_t size = strlen(table->name) + 1 + sizeof(uint16_t);
    size += sizeof(uint16_t);
    for (int i = 0; i < table->column_num; i++) {
        size += strlen(table->structure[i]->name) + 1 + sizeof(uint16_t);
        size += sizeof(column_type);
        size += sizeof(uint8_t);
        size += sizeof(column_function);
        size += sizeof(uint16_t);
        size += table->structure[i]->meta_length;
    }
    return size;
}

void table_serialize(table_t *table, void *destination) {
    table_serialize_off(table, destination, 0);
}

void table_serialize_off(table_t *table, void *destination, size_t offset) {
    uint16_t string_length = strlen(table->name) + 1;
    size_t off = offset;
    memcpy(destination + off, &string_length, sizeof(uint16_t));
    off += sizeof(uint16_t);
    memcpy(destination + off, table->name, string_length);
    off += strlen(table->name) + 1;
    memcpy(destination + off, &table->column_num, sizeof(uint16_t));
    off += sizeof(uint16_t);
    for (int i = 0; i < table->column_num; i++) {
        string_length = strlen(table->structure[i]->name) + 1;
        memcpy(destination + off, &string_length, sizeof(uint16_t));
        off += sizeof(uint16_t);
        memcpy(destination + off, table->structure[i]->name, string_length);
        off += string_length;
        memcpy(destination + off, &table->structure[i]->type, sizeof(column_type));
        off += sizeof(column_type);
        memcpy(destination + off, &table->structure[i]->default_value, sizeof(uint8_t));
        off += sizeof(uint8_t);
        memcpy(destination + off, &table->structure[i]->function, sizeof(column_function));
        off += sizeof(column_function);
        memcpy(destination + off, &table->structure[i]->meta_length, sizeof(uint16_t));
        off += sizeof(uint16_t);
        if (table->structure[i]->meta_length > 0) {
            memcpy(destination + off, table->structure[i]->meta, table->structure[i]->meta_length);
            off += table->structure[i]->meta_length;
        }
    }
}

table_t *table_deserialize(void *source) {
    return table_deserialize_off(source, 0);
}

table_t *table_deserialize_off(void *source, size_t offset) {
    table_t *table = malloc(sizeof(table_t));
    size_t off = offset;
    uint16_t string_length = 0;
    memcpy(&string_length, source + off, sizeof(uint16_t));
    off += sizeof(uint16_t);
    table->name = malloc(string_length);
    memcpy(table->name, source + off, string_length);
    off += string_length;
    memcpy(&table->column_num, source + off, sizeof(uint16_t));
    off += sizeof(uint16_t);
    if (table->column_num > 0) {
        table->structure = malloc(sizeof(column_t *) * table->column_num);
        for (int i = 0; i < table->column_num; i++) {
            table->structure[i] = malloc(sizeof(column_t));
            memcpy(&string_length, source + off, sizeof(uint16_t));
            off += sizeof(uint16_t);
            table->structure[i]->name = malloc(string_length);
            memcpy(table->structure[i]->name, source + off, string_length);
            off += string_length;
            memcpy(&table->structure[i]->type, source + off, sizeof(column_type));
            off += sizeof(column_type);
            memcpy(&table->structure[i]->default_value, source + off, sizeof(uint8_t));
            off += sizeof(uint8_t);
            memcpy(&table->structure[i]->function, source + off, sizeof(column_function));
            off += sizeof(column_function);
            memcpy(&table->structure[i]->meta_length, source + off, sizeof(uint16_t));
            off += sizeof(uint16_t);
            if (table->structure[i]->meta_length > 0) {
                table->structure[i]->meta = malloc(table->structure[i]->meta_length);
                memcpy(table->structure[i]->meta, source + off, table->structure[i]->meta_length);
                off += table->structure[i]->meta_length;
            }
        }
    }
    return table;
}

char *column_type_name(column_t *column) {
    char *ret;
    if (column->type == NUMBER) {
        ret = "TYPE: NUMBER";
    }
    else if (column->type == STRING) {
        ret = "TYPE: STRING";
    }
    else if (column->type == BOOL) {
        ret = "TYPE: BOOL";
    }
    else {
        ret = "TYPE: UNKNOWN";
    }
    return ret;
}

char *column_function_name(column_t *column) {
    char *ret;
    if (column->function == INCREMENTAL) {
        ret = "FUNCTION: INCREMENTAL";
    }
    else if (column->function == NONE) {
        ret = "FUNCTION: NONE";
    }
    else {
        ret = "FUNCTION: UNKNOWN";
    }
    return ret;
}

void table_print(table_t *table) {
    printf("-------------------------------------------------------------\n");
    printf("TABLE %s (\n", table->name);
    for (int i = 0; i < table->column_num; i++) {
        printf("  %s (%s, %s)\n", table->structure[i]->name, column_type_name(table->structure[i]), column_function_name(table->structure[i]));
    }
    printf(");\n\n");
    printf("Data (%i):\n", table->row_num);
    for (int j = 0; j < table->row_num; j++) {
        void *data = table->row_data[j];
        size_t off = 0;
        printf("%i: ", j);
        for (int i = 0; i < table->column_num; i++) {
            column_t *column = table->structure[i];
            if (i == 0) {
                printf("(");
            }
            if (column->type == NUMBER) {
                int num;
                memcpy(&num, data + off, sizeof(int));
                off += sizeof(int);
                printf("%i", num);
            }
            else if (column->type == STRING) {
                uint16_t string_length;
                memcpy(&string_length, data + off, sizeof(uint16_t));
                off += sizeof(uint16_t);
                char *string = malloc(string_length);
                memcpy(string, data + off, string_length);
                off += string_length;
                printf("%s", string);
                free(string);
            }
            else if (column->type == BOOL) {
                uint8_t byte;
                memcpy(&byte, data + off, sizeof(uint8_t));
                off += sizeof(uint8_t);
                if (byte == 0) {
                    printf("FALSE");
                }
                else {
                    printf("TRUE");
                }
            }
            if (i == table->column_num - 1) {
                printf(")\n");
            }
            else {
                printf(", ");
            }
        }
    }
    printf("-------------------------------------------------------------\n");
}

size_t table_data_size(table_t *table) {
    size_t size = 0;
    for (int i = 0; i < table->row_num; i++) {
        size += table->row_size[i] + sizeof(uint32_t);
    }
    return size;
}

table_t *table_create(char *name) {
    table_t *table = malloc(sizeof(table_t));
    table->name = name;
    table->column_num = 0;
    table->row_num = 0;
    table->structure = NULL;
    table->row_data = NULL;
    return table;
}

void table_free(table_t *table) {
    for (int i = 0; i < table->column_num; i++) {
        free(table->structure[i]->meta);
        free(table->structure[i]);
    }
    for (int i = 0; i < table->row_num; i++) {
        free(table->row_data[i]);
    }
    free(table->row_data);
    free(table->row_size);
    free(table->structure);
    free(table);
}