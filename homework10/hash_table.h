#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define TABLE_SIZE 100000 // Size of the Hash Table
#define MAX_BUF 6144
#define MAX_VALUE 10

// Структура для элемента хэш-таблицы
typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

typedef struct Max_url
{
    char key[MAX_BUF];
    int value;
}Max_url;

typedef struct Max   //структура для потока
{
    char path[PATH_MAX];
    Max_url url[MAX_VALUE];
    Max_url refer[MAX_VALUE];
    uint64_t all_url;
}Max;

// Структура хэш-таблицы
typedef struct {
    Node *buckets[TABLE_SIZE];
} HashTable;

// Создание нового элемента
Node *create_node(const char *key, int value);
// Инициализация хэш-таблицы
void init_table(HashTable *table);
// Вставка элемента в хэш-таблицу
void insert(HashTable *table, const char *key, int value);
// Поиск элемента в хэш-таблице
int search(HashTable *table, const char *key, int *value);
// Удаление элемента из хэш-таблицы
void delete(HashTable *table, const char *key) ;
// Освобождение памяти хэш-таблицы
void free_table(HashTable *table);
// Печать хэш-таблицы (для отладки)
void print_table(HashTable *table);
unsigned long long search_url(HashTable *table, Max_url *max);

#endif // HASH_TABLE_H
