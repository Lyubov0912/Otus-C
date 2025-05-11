#include <stdio.h>
#include <stdlib.h>

static const char* int_format = "%ld ";
static const char* empty_str = "";

static long data[] = {4, 8, 15, 16, 23, 42};
static size_t data_length = sizeof(data) / sizeof(long);

typedef struct elem
{
    long value;
    struct elem *next;
}Element;

static void print_int(long value) {
    printf(int_format, value);
    fflush(NULL);
}

static int p(long value) {
    return value & 1;
}

static Element* add_element(long value, Element* next) {
    Element* elem = malloc(sizeof(Element));
    if (elem == NULL)
        abort();

    elem->value = value;
    elem->next = next;
    return elem;
}

static void m(Element* list, void (*f)(long)) {
    if (list == NULL) return;

    long value = list->value;
    void* next = list->next;
    f(value);
    m(next, f);
}

static void f(Element* list, Element **next, int (*p)(long)) {
    if (list == NULL) return;

    long value = list->value;
    if (p(value))
        *next = add_element(list->value, *next);
    f(list->next, next, p);
}

int main() {
    Element* list = NULL;

    for (size_t i = 0; i < data_length; i++)
         list = add_element(data[i], list);

    m(list, print_int);

    puts(empty_str);

    Element* result = NULL;
    f(list, &result, p);

    m(result, print_int);

    puts(empty_str);

    return 0;
}

