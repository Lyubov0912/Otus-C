#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

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

    char log[100];
    sprintf(log, " Func p return %ld\n", value & 1);
    writeLog(ll_debug, log, __FUNCTION__, __LOGPLACE__);
    return value & 1;
}

static Element* add_element(long value, Element* next) {
    Element* elem = malloc(sizeof(Element));
    if (elem == NULL)
    {
        writeLog(ll_error, " No malloc for add element\n", __FUNCTION__, __LOGPLACE__);
        abort();
    }

    char log[50];
    sprintf(log, " Add value = %ld\n", value);
    writeLog(ll_debug, log, __FUNCTION__, __LOGPLACE__);

    elem->value = value;
    elem->next = next;
    return elem;
}

static void m(Element* list, void (*f)(long)) {
    if (list == NULL)
    {
        writeLog(ll_error, " List = NULL\n", __FUNCTION__, __LOGPLACE__);
        return;
    }
    long value = list->value;
    void* next = list->next;
    f(value);
    writeLog(ll_message, " TODO: replace recursion\n", __FUNCTION__, __LOGPLACE__);
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

    int mas[4] = {1,1,1,0};   //устанавливаем уровни логирования для вывода в файл
    setLevel(mas);
    setPath("/tmp/file.log");  //устанавливаем путь к файлу логирования
    logger();                  //запускаем лонгер

    Element* list = NULL;

    writeLog(ll_warning, " array out of range\n", __FUNCTION__, __LOGPLACE__);

    for (size_t i = 0; i < data_length; i++)
         list = add_element(data[i], list);

    m(list, print_int);

    puts(empty_str);

    Element* result = NULL;
    f(list, &result, p);

    m(result, print_int);

    puts(empty_str);

    stopLogThread();

    return 0;
}

