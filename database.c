#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char *new_replace_char(char *string, char original, char replace) {
    char *ret = malloc(strlen(string) + 1);
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == original) {
            ret[i] = replace;
        }
        else {
            ret[i] = string[i];
        }
    }
    ret[strlen(string)] = '\0';
    return ret;
}

void db_store(database_t *db) {
    struct stat st;
    char db_file[strlen(db->name) + 10];
    char *name = new_replace_char(db->name, ' ', '_');
    sprintf(db_file, "%s%s%s", "./data/", name, ".db");
    if (stat("./data/", &st) == -1) {
        mkdir("./data/", 0777);
    }
    int db_fp = open(db_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    void *data = malloc(db_size(db));
    uint16_t string_length = strlen(db->name) + 1;
    memcpy(data, &string_length, sizeof(uint16_t));
    size_t off = sizeof(uint16_t);
    memcpy(data + off, db->name, string_length);
    off += string_length;
    memcpy(data + off, &db->table_num, sizeof(uint16_t));
    off += sizeof(uint16_t);
    for (int i = 0; i < db->table_num; i++) {
        table_serialize_off(db->tables[i], data, off);
        off += table_size(db->tables[i]);
    }
    write(db_fp, data, off);
    ftruncate(db_fp, off);

    free(data);
    close(db_fp);
    char db_dir[strlen(db->name) + 8];
    sprintf(db_dir, "%s%s%s", "./data/", name, "/");
    if (stat(db_dir, &st) == -1) {
        mkdir(db_dir, 0777);
    }
    free(name);
    for (int i = 0; i < db->table_num; i++) {
        table_t *table = db->tables[i];
        char table_file[strlen(table->name) + strlen(db_dir) + 3];
        name = new_replace_char(table->name, ' ', '_');
        sprintf(table_file, "%s%s%s", db_dir, name, ".db");
        free(name);
        int table_fp = open(table_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        size_t table_size = table_data_size(table);
        off = 0;
        void *table_data = malloc(table_size);
        for (int j = 0; j < table->row_num; j++) {
            uint32_t rs = table->row_size[j];
            memcpy(table_data + off, &rs, sizeof(uint32_t));
            off += sizeof(uint32_t);
            memcpy(table_data + off, table->row_data[j], rs);
            off += rs;
        }
        write(table_fp, table_data, off);
        ftruncate(table_fp, off);
        free(table_data);
        close(table_fp);
    }
}

void db_add_table(database_t *db, char *name) {
    table_t *table = table_create(name);
    if (db->tables == NULL) {
        db->tables = malloc(sizeof(table_t *));
        db->tables[0] = table;
        db->table_num++;
    }
    else {
        db->table_num++;
        db->tables = realloc(db->tables, sizeof(table_t *) * db->table_num);
        db->tables[db->table_num - 1] = table;
    }
}

table_t *db_get_table(database_t *db, char *name) {
    for (int i = 0; i < db->table_num; i++) {
        if (strcmp(db->tables[i]->name, name) == 0) {
            return db->tables[i];
        }
    }
    return NULL;
}

size_t db_size(database_t *db) {
    uint16_t string_length = 0;
    size_t db_size = sizeof(uint16_t);
    string_length = strlen(db->name) + 1;
    db_size += string_length;
    db_size += sizeof(uint16_t);
    for (int i = 0; i < db->table_num; i++) {
        db_size += table_size(db->tables[i]);
    }
    return db_size;
}

database_t *db_create(char *name) {
    database_t *db = malloc(sizeof(database_t));
    db->name = name;
    db->table_num = 0;
    db->tables = NULL;
    return db;
}

void db_free(database_t *db) {
    for (int i = 0; i < db->table_num; i++) {
        table_free(db->tables[i]);
    }
    free(db->tables);
    free(db);
}