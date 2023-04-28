#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 30
#define QUE_SIZE 5
void error_handling(char *msg);

int main(int argc, char *argv[])
{
    int                serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t          clnt_addr_size;
    struct timeval     timeout;
    fd_set             reads, cpy_reads;
    int                fd_max, str_len, fd_num, ii;
    char               buf[BUF_SIZE];

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    // 소켓 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");
    // sockaddr_in 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    // bind()
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    // listen()
    if (listen(serv_sock, QUE_SIZE) == -1)
        error_handling("listen() error");
    // fd_set 초기화
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;
    while (1) {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;
        // select
        fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);
        if (fd_num == -1)
            break;
        if (fd_num == 0)
            continue;
        for (int i = 0; i < fd_max + 1; i++) {
            if (FD_ISSET(i, &cpy_reads)) {
                if (i == serv_sock) { // connection requests
                    clnt_addr_size = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr,
                                       &clnt_addr_size);
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    printf("connected client: %d\n", clnt_sock);
                }
                else { // read
                    str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0) {
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d\n", i);
                    }
                    else {
                        write(i, buf, str_len); // echo
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
