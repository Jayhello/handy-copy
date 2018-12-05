//
// Created by root on 11/29/18.
//

#include <iostream>
#include <arpa/inet.h>
#include "handy/handy.h"

using namespace handy;
using namespace std;

/*
 * server side: Test tcp send buf full.
 *
 */
void test_full_sock_buf(){
//    string ip = "172.25.53.26";
    string ip = "127.0.0.1";
    uint16_t port = 8888;

    int svrFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(svrFd<0, "create socket error:%d, %s", errno, strerror(errno));
    net::setReuseAddr(svrFd);

    Ip4Addr addr(ip, port);

    int ret = bind(svrFd, (struct sockaddr*)&addr.getAddr(),
                   sizeof(addr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    ret = listen(svrFd, 100);
    fatalif(ret<0, "listen socket failed, %d, %s", errno, strerror(errno));

    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cFd = accept(svrFd, (struct sockaddr *) &raddr, &rsz);

    fatalif(cFd<0, "accept failed");

    Ip4Addr tmp(raddr);
    info("server fd:%d, client fd: %d, accept a connection from %s, but I will sleep 100s",
         svrFd, cFd, tmp.toString().c_str());

    char buf[1025 * 10];

//    while(1){
//        memset(buf, '0', sizeof buf);
//
//        int len = recv(svrFd, buf, sizeof buf, 0);
//
//        info("recv len: %d, erron: %d, %s", len, errno, strerror(errno));
//        sleep(1);
//    }
    sleep(100);
}

//#define exit_if(r, ...)                                                                          \
//    if (r) {                                                                                     \
//        printf(__VA_ARGS__);                                                                     \
//        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno)); \
//        exit(1);                                                                                 \
//    }
//
//void test_full_sock_buf_1(){
//
//    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
//    exit_if(listenfd<0, "create socket error");
//
//    short port = 8000;
//    struct sockaddr_in addr;
//    memset(&addr, 0, sizeof addr);
//    addr.sin_family = AF_INET;
//    addr.sin_port = htons(port);
//    addr.sin_addr.s_addr = INADDR_ANY;
//
//    int r = ::bind(listenfd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
//    exit_if(r<0, "bind socket error");
//
//    r = listen(listenfd, 100);
//    exit_if(r<0, "listen socket error");
//
//    struct sockaddr_in raddr;
//    socklen_t rsz = sizeof(raddr);
//    int cfd = accept(listenfd, (struct sockaddr *) &raddr, &rsz);
//    exit_if(cfd<0, "accept socket error");
//
//    sockaddr_in peer;
//    socklen_t alen = sizeof(peer);
//    getpeername(cfd, (sockaddr *) &peer, &alen);
//
//    printf("accept a connection from %s:%d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
//
//    printf("but now I will sleep 15 second, then exit");
//    sleep(15);
//}


int main(){

    test_full_sock_buf();


    return 0;
}