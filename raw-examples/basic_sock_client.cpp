//
// Created by root on 11/29/18.
//

#include <iostream>
#include "handy/handy.h"
#include <fcntl.h>
using namespace handy;
using namespace std;

/*
 * server side: Test tcp send buf full.
 *
 */
void test_full_sock_buf(){
    string ip = "172.25.53.26";
    uint16_t port = 8888;
    Ip4Addr addr(ip, port);

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(sockFd<0, "create socket error:%d, %s", errno, strerror(errno));

    int ret = connect(sockFd, (struct sockaddr*)&addr.getAddr(), sizeof(addr.getAddr()));
    fatalif(ret < 0, "connect failed %d %s", errno, strerror(errno));

    info("connect to server success, sock fd: %d", sockFd);

    net::setNonBlock(sockFd);

    string msg(1024 * 1000, 'a');
    for (int i = 0; i < 1000; ++i) {
        int len = send(sockFd, msg.c_str(), msg.size(), 0);

        info("loop: %d, send len: %d, erron: %d, %s", i, len, errno, strerror(errno));
        sleep(1);
    }
}

//#define exit_if(r, ...)                                                                          \
//    if (r) {                                                                                     \
//        printf(__VA_ARGS__);                                                                     \
//        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno)); \
//        exit(1);                                                                                 \
//    }
//
//void setNonBlock(int fd) {
//    int flags = fcntl(fd, F_GETFL, 0);
//    exit_if(flags < 0, "fcntl failed");
//    int r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
//    exit_if(r < 0, "fcntl failed");
//}
//
//
//void test_full_sock_buf_1(){
//    short port = 8000;
//    struct sockaddr_in addr;
//    memset(&addr, 0, sizeof addr);
//    addr.sin_family = AF_INET;
//    addr.sin_port = htons(port);
//    addr.sin_addr.s_addr = INADDR_ANY;
//
//
//    int fd = socket(AF_INET, SOCK_STREAM, 0);
//    exit_if(fd<0, "create socket error");
//
//    int ret = connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
//    exit_if(ret<0, "connect to server error");
//    setNonBlock(fd);
//
//    printf("connect to server success");
//
//    const int LEN = 1024 * 1000;
//    char msg[LEN];  // 1MB data
//    memset(msg, 'a', LEN);
//
//    for (int i = 0; i < 1000; ++i) {
//        int len = send(fd, msg, LEN, 0);
//        printf("send: %d, erron: %d, %s \n", len, errno, strerror(errno));
//        sleep(1);
//    }
//
//}


int main(){
    test_full_sock_buf();


    return 0;
}