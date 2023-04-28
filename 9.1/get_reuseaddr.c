#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
void error_handling(char *msg);

int main(int argc, char *argv[])
{
    int sock;
    int snd_opt, rcv_opt, state;
    socklen_t len;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error!");
    
    len = sizeof(snd_opt);
    state = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&snd_opt, &len);
    if (state)
        error_handling("getsockopt() error");
    
    len = sizeof(rcv_opt);
    state = getsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&rcv_opt, &len);
    if (state)
        error_handling("getsockopt() error");

    printf("Input buffer size: %d\n", rcv_opt);
    printf("Output buffer size: %d\n", snd_opt);
}

void    error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
