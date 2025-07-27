#include "hash_table.h"
#include <signal.h>
#include <pthread.h>
#include <dirent.h>
#include <semaphore.h>
#include <inttypes.h>

pthread_barrier_t barrier;

void *ReadThread(void* data)
{
    FILE *f;
    Max*max = (Max*)data;
    printf("%s\n",max->path);
    f= fopen(max->path, "r");
    if (f == NULL) {
        return NULL;
    }
    HashTable table, table_refer;
    init_table(&table);
    init_table(&table_refer);
    int i = 0;
    int i_size=0;
    char * s;
    char buf[MAX_BUF];
    char url[MAX_BUF];
    memset(url, 0, sizeof(url));
    memset(buf, 0, sizeof(buf));
    while (!feof(f))
    {
        fgets(buf, MAX_BUF, f);   //ищем начало url
        s = strstr(buf, "GET");
        i = 0;
        if (s != NULL)
        {
            i  = s - buf +4;
        }
        else
        {
            s = strstr(buf, "HEAD");
            if (s != NULL)
                i = s - buf + 5;
        }
        if ( i>0 )
        {
            int j = 0;
            while (i < MAX_BUF && j < MAX_BUF && buf[i] != ' ')  //собираем url
            {
                url[j] = buf[i];
                i++;
                j++;
            }
            while (i < MAX_BUF && buf[i] !='"')
            {
                i++;
            }
            i += 6;
            char s_url[10];
            j = 0;
            while (i < MAX_BUF && j < MAX_BUF && buf[i] != ' ')  //считываем размер url
            {
                s_url[j] = buf[i];
                i++;
                j++;
            }
            i_size = atoi(s_url);
            insert(&table, url, i_size);      //добавляем url в хэш таблицу
            memset(s_url, 0, sizeof(s_url));
            memset(url, 0, sizeof(url));
            j = 0;
            i += 2;
            while (i < MAX_BUF && j < MAX_BUF && buf[i] != '"')  //собираем refer
            {
                url[j] = buf[i];
                i++;
                j++;
            }
            if (strcmp(url, "-") != 0)
                insert(&table_refer, url, 1);     //добавляем refer в хэш таблицу
        }
        memset(url, 0, sizeof(url));
        memset(buf, 0, sizeof(buf));
    }

    max->all_url = search_url(&table, max->url);    //ищем самые тяжелые url
    search_url(&table_refer, max->refer);           //ищем самые часто встречающиеся refer

    free_table(&table);
    free_table(&table_refer);
    fclose(f);
    pthread_barrier_wait(&barrier);
    return NULL;
}

int count_file(char *path)
{
    int count = 0;
    DIR *dir = opendir(path);
    if (dir == NULL) return -1;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
        count++;
    closedir(dir);
    return (count-2);
}

void search_max(Max_url* mas, Max_url *mas2)  //получение массива наиболее тяжелых url(часто встречающихся refer) из двух массивов
{
    for(int i=0;i<MAX_VALUE;i++)
    {
        for (int j=0; j<MAX_VALUE;j++)
        {
            if(mas[j].value < mas2[i].value)
            {
                for(int k=(MAX_VALUE-1); k>j;k--)
                {
                    memset(mas[k].key, 0, sizeof(mas[k].key));
                    memcpy(mas[k].key, mas[k-1].key, strlen(mas[k-1].key));
                    mas[k].value = mas[k-1].value;
                }
                mas[j].value = mas2[i].value;
                memset(mas[j].key, 0, strlen(mas[j].key));
                memcpy(mas[j].key, mas2[i].key, strlen(mas2[i].key));
                break;;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    char path[PATH_MAX];
    char fullpath[PATH_MAX];    
    memset(path, 0, sizeof(path));
    int threads_count = 1;
    if (argc >1)
    {
        memcpy(path, argv[1], strlen(argv[1]));
        if (argc>2)
            threads_count = atoi(argv[2]);
    } else return -1;

    int leftover = count_file(path);
    int count = leftover;
    Max max[count];
    int k = 0;

    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("No such directory: %s", path);
        return -1;
    }
    struct dirent *entry;

    pthread_barrier_init(&barrier, NULL, threads_count);
    pthread_t threads[threads_count];
    entry = readdir(dir);
    while (entry != NULL)        //пока в директории есть файлы
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            entry = readdir(dir);
            continue;
        }
        int i = 0;
        if (leftover < threads_count)     //если количество оставшихся для обработки файлов меньше заданного количества потоков
        {
            threads_count = leftover;
            pthread_barrierattr_destroy(&barrier);
            pthread_barrier_init(&barrier, NULL, leftover);
        }
        while ((i<threads_count)&&(entry!=NULL))       //создаем заданное количество потоков
        {
            memset(fullpath, 0, sizeof(fullpath));
            sprintf(max[k].path,"%s%s",path,entry->d_name);
            if (pthread_create(&threads[i], NULL, ReadThread, (void*)&max[k]) !=0) //передаем структуру с именем файла для обработки и массивы для получения результата с url и refer
            {
                printf("error\n");
                exit(1);
            }
            k++;
            entry = readdir(dir);

           i++;
           leftover --;
        }
        for (int j=0; j<threads_count; j++)        //ждем завершения созданных потоков, если файлы для обработки еще остались, то идем на след круг
            pthread_join(threads[j], NULL);

    }

    uint64_t all = max[0].all_url;
    for (int i=1; i<count;i++)
    {
        all += max[i].all_url;
        search_max(max[0].url, max[i].url);
        search_max(max[0].refer, max[i].refer);
    }

    for(int i=0; i<MAX_VALUE; i++)
    {
        printf("%d url: ", i);
        printf("%s ", max[0].url[i].key);
        printf("(%u)\nrefer: ", max[0].url[i].value);
        printf("%s ", max[0].refer[i].key);
        printf("(%u)\n\n", max[0].refer[i].value);
    }
    printf("All bytes url: %" PRIu64 "\n", all);
    pthread_barrierattr_destroy(&barrier);
    closedir(dir);

    return 0;

}
