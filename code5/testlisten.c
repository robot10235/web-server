#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static bool stop = false;

static void handle_term(int sig) {
    stop = true;
}

int main(int argc, char* argv[]) {
    signal(SIGTERM, handle_term);

    if(argc <= 3) {
        printf("usage: %s ip_address port_number backlog\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // construct ipv4 address 
    // ip / port needed to be transform to big-endian / network byte order
    struct sockaddr_in address;
    bzero(&address, sizeof(address));       // init address with zeros
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sock, backlog);    // backlog is the largest size of listen
    assert(ret != -1);

    // wait for connection until SIGTERM end it
    // can't figure out how continue the connection
    while(!stop) {
        sleep(1);
    }

    close(sock);
    return 0;
}