#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
#define QUE_SIZE 5
void error_handling(char *msg);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
    int                 serv_sock, clnt_sock;
    struct sockaddr_in  serv_addr, clnt_addr;
    socklen_t           clnt_addr_size;
    
    pid_t               pid;
    struct sigaction    act;
    int                 str_len, state;
    char                buff[BUF_SIZE];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    // 시그널 설정
    act.__sigaction_u.__sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);
    // 연결용 서버 소켓 생성하기
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
    // sockaddr_in 초기화하기
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    // binding하기
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    // listen 하기
    if (listen(serv_sock, QUE_SIZE) == -1)
        error_handling("listen() error");
    // accept 하기
    while (1)
    {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            continue ;
        else
            puts("new client connected...");
        pid = fork();
        if (pid == -1)
        {
            close(clnt_sock);
            continue ;
        }
        else if (pid == 0) // child process
        {
            close(serv_sock);
            while ((str_len = read(clnt_sock, buff, BUF_SIZE)))
            {
                if (str_len == -1)
                {
                    close(clnt_sock);
                    exit(1);
                }
                write(clnt_sock, buff, str_len);
            }
            close(clnt_sock);
            puts("client disconnected...");
            return 0;
        }
        else
            close(clnt_sock);
    }
    close(serv_sock);
    return 0;
}

void    read_childproc(int sig)
{
    pid_t   pid;
    int     status;

    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}

void    error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}