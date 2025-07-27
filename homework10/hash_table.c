#include "hash_table.h"


// Хэш-функция (простая для примера)
unsigned int hash(const char *key) {
    unsigned int hash_value = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash_value = hash_value * 31 + key[i];
    }
    return hash_value % TABLE_SIZE;
}

// Создание нового элемента
Node *create_node(const char *key, int value) {
    Node *node = (Node *)malloc(sizeof(Node));
    if(node ==NULL)
    {
       return NULL;
    }
    node->key = strdup(key);
    if (node->key == NULL)
    {
        return NULL;
    }
    node->value = value;
    node->next = NULL;
    return node;
}

// Инициализация хэш-таблицы
void init_table(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
}

// Вставка элемента в хэш-таблицу
void insert(HashTable *table, const char *key, int value) {
    unsigned int index = hash(key);
    Node *new_node = create_node(key, value);

    // Если нет такого элемента, просто вставляем
    if (table->buckets[index] == NULL) {
        table->buckets[index] = new_node;
    } else
    {
        Node *current = table->buckets[index];
        while (current != NULL)
        {
            if (strcmp(current->key, key) == 0)
            {
                table->buckets[index]->value += value;
                free(new_node->key);
                free(new_node);
                return;
            }
            else
            {
                if (current->next == NULL)
                {
                    current->next = new_node;
                    return;
                }
                current = current->next;
            }
        }
    }
}

// Поиск элемента в хэш-таблице
int search(HashTable *table, const char *key, int *value) {
    unsigned int index = hash(key);
    Node *current = table->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            *value = current->value;
            return 1; // Найден
        }
        current = current->next;
    }

    return 0; // Не найден
}

// Освобождение памяти хэш-таблицы
void free_table(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = table->buckets[i];
        while (current != NULL) {
            Node *temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
}

//Наити самые тяжелые url
unsigned long long search_url(HashTable *table, Max_url* max)
{
    unsigned int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = table->buckets[i];
        while (current != NULL) {
            for (int j=0; j<MAX_VALUE; j++)
            {
                count += current->value;
                if (current->value > max[j].value)
                {
                    for(int k=(MAX_VALUE-1); k>j;k--)
                    {
                        memset(max[k].key, 0, sizeof(max[k].key));
                        memcpy(max[k].key, max[k-1].key, strlen(max[k-1].key));
                        max[k].value = max[k-1].value;
                    }
                    memcpy(&max[j].key, current->key, strlen(current->key));
                    max[j].value = current->value;
                    break;
                }
            }
            current = current->next;
        }
    }
    return count;
}
