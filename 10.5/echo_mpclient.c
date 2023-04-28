#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void    error_handling(char *msg);
void    signal_handling(int sig);
void    read_routine(int sock, char *buff);
void    write_routine(int sock, char *buff);

int main(int argc, char *argv[])
{
    int sock;
    pid_t pid;
    char buff[BUF_SIZE];
    struct sockaddr_in sock_addr;
    struct sigaction    act;
    int                 state;

    if (argc != 3)
    {
        printf("Usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }
    // signal 설정
    act.__sigaction_u.__sa_handler = signal_handling;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);
    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");
    // 초기화
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(argv[1]);
    sock_addr.sin_port = htons(atoi(argv[2]));
    // 연결
    if (connect(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
        error_handling("connect() error");
    pid = fork();
    if (pid == 0)
        write_routine(sock, buff);
    else
        read_routine(sock, buff);
    close(sock);
    return 0;
}

void read_routine(int sock, char *buff)
{
    int str_len;

    while (1)
    {
        str_len = read(sock, buff, BUF_SIZE);
        if (str_len == -1)
        {
            close(sock);
            exit(2);
        }
        if (str_len == 0)
            return ;
        buff[str_len] = 0;
        printf("Message from server: %s\n", buff);
    }
}

void    write_routine(int sock, char *buff)
{

    while (1)
    {
        fgets(buff, BUF_SIZE, stdin);
        if (!strcmp(buff, "q\n") || !strcmp(buff, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return ;
        }
        write(sock, buff, strlen(buff));
    }
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void    signal_handling(int sig)
{
    pid_t   pid;
    int     status;
    pid = waitpid(-1, &status, WNOHANG);
}