#ifndef LOGGER_H_
#define LOGGER_H_

#define __FILENAME__ (strrchr(__FILE__,'/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)
#define __LOGPLACE__ (S__LINE__)

#define MAXLOGLEVEL 4

enum LogLevel
{
    ll_default,
    ll_error, ll_debug, ll_warning, ll_message
};


//запуск потока логера
int logger();


//запись сообщения в файл, параметры: lvl - уровень логирования сообщения,
//                                    mess - сообщение,
//                                    func - функция вызова засиси в лог(__FUNCTION__),
//                                    line - строка вызова записи в лог(__FUNCTION__)
void writeLog(int lvl, char * mess, const char *func, char *line);


//установка уровней вывода в лог, lvl - массив из MAXLOGLEVEL элементов, 
//                                      lvl[0](ll_error), 
//										lvl[1](ll_debug),
//										lvl[2](ll_warning),
//										lvl[3](ll_message),
//										0 - не выводить в лог файл.
void setLevel(int *lvl);


//задание файла логирования, buf - путь и имя файла логирования
void setPath(char *buf);


//завершение потока логирования
void stopLogThread();

#endif /* LOGGER_H_*/
