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


/*
 * Test socket fd read, write ready condition.
 *
 */
void test_connect_fd_write_read_cond(){
//    string ip = "220.181.57.216";
    string ip = "172.25.53.26";
//    string ip = "127.0.0.1";
    uint16_t port = 8888;
    Ip4Addr addr(ip, port);

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(sockFd<0, "create socket error:%d, %s", errno, strerror(errno));

    net::setNonBlock(sockFd);

    int ret = connect(sockFd, (struct sockaddr*)&addr.getAddr(), sizeof(addr.getAddr()));
//    fatalif(ret < 0, "connect failed %d %s", errno, strerror(errno));

    info("create socket %d, connect ret: %d, errno: %d, %s", sockFd, ret, errno, strerror(errno));

    fd_set rFds, wFds;
    timeval tv;

    int err = 0;
    unsigned int errLen = sizeof err;

    while(1){
        tv.tv_sec = 10;

        FD_ZERO(&rFds);
        FD_SET(sockFd, &rFds);

        FD_ZERO(&wFds);
        FD_SET(sockFd, &wFds);

        int num = select(sockFd + 1, &rFds, &wFds, NULL, &tv);

        if(num < 0){
            error("select error, errno: %d, %s", errno, strerror(errno));
            break;
        } else if(0 == num){
            info("select timeout");
        } else{

            ret = getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &errLen);
            warn("getsockopt(SO_ERROR): ret: %d, err: %d, errno:%d, %s", ret, err, errno, strerror(errno));

//            if (getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &errLen) == -1) {
//                warn("getsockopt(SO_ERROR): %s", strerror(errno));
//                close(sockFd);
//                break;
//            }

            if(FD_ISSET(sockFd, &rFds)){
                info("read fd ready, err: %d, errno: %d, %s", err, errno, strerror(errno));
            }

            if(FD_ISSET(sockFd, &wFds)){
                info("write fd ready, err: %d, errno: %d, %s", err, errno, strerror(errno));
            }

            sleep(3);
        }

    }

}


void test_get_sock(){
    int err = 0, ret = 0;
    unsigned int errLen = sizeof err;
    ret = getsockopt(4, SOL_SOCKET, SO_ERROR, &err, &errLen);

    info("getsockopt: ret: %d, err:%d, errno:%d, :%s", ret, err, errno, strerror(errno));
    // getsockopt: ret: -1, err:0, errno:9, :Bad file descriptor

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errLen);
    info("fd: %d, ret: %d, err:%d, errno:%d, :%s", fd, ret, err, errno, strerror(errno));
    // fd: 3, ret: 0, err:0, errno:9, :Bad file descriptor

    // receive buf size, 32KB
    int bufLen=32*1024;
    setsockopt(fd, SOL_SOCKET,SO_RCVBUF,(const char*)&bufLen,sizeof(int));


    int to=1000;//1 second
    //send timeout
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&to, sizeof(to));
    //receive timeout
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&to, sizeof(to));

    int nBuf=0;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&nBuf, sizeof(nBuf));
}

int main(){
//    test_full_sock_buf();
//    test_connect_fd_write_read_cond();
    test_get_sock();

    std::function<void()> fun;

    return 0;
}