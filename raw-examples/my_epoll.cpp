//
// Created by root on 18-11-21.
//

/*
 * Mainly test for server end.
 */

#include <sys/socket.h>

#include <vector>
#include <iostream>
#include <netinet/in.h>

#include <handy/handy.h>
#include <arpa/inet.h>

using namespace handy;
using namespace std;

/*
 * Note: Simple test for max socket num.
 */
void testMaxSockFd(){
    vector<int> vecFd(40000);

    for (int i = 0; i < vecFd.size(); ++i) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        fatalif(fd<0, "create sock %d, errno: %d, %s:",i+2, errno, strerror(errno));
        // create sock 4093, errno: 24, Too many open files:

        vecFd[i] = fd;
    }
}



sockaddr_in getSockAddr(const string& ip, uint16_t port){
    sockaddr_in localAddr;
    bzero(&localAddr,sizeof(localAddr));

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(ip.data());
    localAddr.sin_port = htons(port);

    return localAddr;
}

auto lb_process_client = [](int fd){
    int count = 0;
    char buf[200];
    memset(buf, '\0', 200);

    for (int i = 0; i < 3; ++i) {
        string msg = "server msg: " + to_string(count++);
        int len = send(fd, (void*)msg.data(), msg.size(), 0);
        if(len < 0){
            info("send error, peer close fd");
            break;
        }

        info("send %d", len);

        memset(buf, '\0', 200);
        len = recv(fd, buf, 200, 0);

        if(len < 0){
            info("recv error, peer close fd");
            break;
        }

        info("recv %s, len %d", buf, len);
        sleep(2);
    }
    
    close(fd);
    info("process ending, %d", fd);
};


void testServer(){
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(sockFd < 0, "socket failed %d %s", errno, strerror(errno));

    string ip = "127.0.0.1";
    uint16_t port = 8000;
    Ip4Addr  localAddr(ip, port);

    int ret = bind(sockFd, (struct sockaddr*)&localAddr.getAddr(), 
                   sizeof(localAddr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    ret = listen(sockFd, 100);
    fatalif(ret < 0, "listen socket failed %d %s", errno, strerror(errno));

    info("now i will sleep 100S");
    sleep(100);

    ThreadPool tp(5);

    while(true){
        struct sockaddr_in raddr;
        socklen_t rsz = sizeof(raddr);
        int cfd = accept(sockFd, (struct sockaddr *) &raddr, &rsz);
        fatalif(cfd < 0, "accept failed");
        Ip4Addr tmp(raddr);
        
        info("accept a connection from %s", tmp.toString().c_str());
        tp.addTask(std::bind(lb_process_client, cfd));
    }

}


int main(){
    testMaxSockFd();

//    testServer();

    return 0;
}