#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "signal.h"

static int sock;

static void sig_usr(int signo)
{
    if (signo == SIGINT)
    {
        close(sock);
    }
}

int main(int argc, char *argv[])
{
    int msgsock;
    struct sockaddr_un server;
    char buf[1024];
    char bufname[64];
    char buffile[255];
    FILE *f, *file_size;
    int size;
    f = fopen("socket.cfg", "r");
    if (!f)
    {
        perror("can not open file");
        return -1;
    }
    fgets(bufname,sizeof(bufname),f);
    size = strlen(bufname);
    bufname[size-1]='\0';
    printf("%s\n", bufname);
    fgets(buffile,sizeof(buffile),f);
    size = strlen(buffile);
    buffile[size-1]='\0';
    printf("%s\n", buffile);
    fclose(f);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("can not create socket");
        exit(1);
    }

    if (signal(SIGINT, sig_usr) == SIG_ERR)
        perror("signal SIGINT");

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, bufname);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)))
    {
        perror("can not bind socket");
         exit(1);
    }
    printf("Socket has name %s\n", server.sun_path);
    listen(sock, 5);

    if (argc >1)
    {
        if (strcmp(argv[1],"-d") == 0)
        {
            pid_t pid;
            pid = fork();
            if (pid < 0)
            {
                printf("Ошибка вызова fork");
                exit(0);
            }
            if (pid > 0)
            {
                printf("Родитель завершается");
                exit(1);
            }
            if (setsid() < 0)
            {
                exit(1);
            }
            chdir("/");
            umask(0);
            close(0);
            close(1);
            close(2);

            for (;;)
            {
                msgsock = accept(sock, 0, 0);
                if (msgsock == -1)
                {
                    break;
                }
                else
                {
                    file_size = fopen(buffile, "r");
                    if (!file_size)
                        {
                            close(msgsock);
                            close(sock);
                            unlink(bufname);
                            exit(1);
                        }
                    fseek(file_size,0, SEEK_END);
                    size = ftello(file_size);
                    fclose(file_size);
                    sprintf(buf, "%d", size);
                    send(msgsock, buf, strlen(buf), 0);
                }
                close(msgsock);
             }

        }
         close(sock);
         unlink(bufname);
         exit(1);
    }
        for (;;)
        {
            msgsock = accept(sock, 0, 0);
            if (msgsock == -1)
            {
                perror("accept");
                break;
            }
            else
            {
                printf("connect client\n");
                file_size = fopen(buffile, "r");
                if (!file_size)
                {
                    perror("can not open file");
                    close(msgsock);
                    close(sock);
                    unlink(bufname);
                    exit(1);
                }
                fseek(file_size,0, SEEK_END);
                size = ftello(file_size);
                fclose(file_size);
                sprintf(buf, "%d", size);
                printf("size file = %s\n", buf);
                send(msgsock, buf, strlen(buf), 0);
                printf("Send size file\n");
            }
            close(msgsock);
            printf("disconnect client\n");
        }
        close(sock);
        unlink(bufname);
    return -1;
}
