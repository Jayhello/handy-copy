//
// Created by root on 18-11-21.
//

#include <sys/socket.h>

#include <iostream>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>

#include <handy/handy.h>

using namespace handy;
using namespace std;

sockaddr_in getSockAddr(const string& ip, uint16_t port){
    sockaddr_in localAddr;
    bzero(&localAddr,sizeof(localAddr));

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(ip.data());
    localAddr.sin_port = htons(port);

    return localAddr;
}

void testClient(){
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(sockFd < 0, "socket failed %d %s", errno, strerror(errno));


    string ip = "127.0.0.1";
    uint16_t clientPort = 8888;
    Ip4Addr localAddr(ip, clientPort);

    int ret = bind(sockFd, (struct sockaddr*)&localAddr.getAddr(), sizeof(localAddr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    uint16_t serverPort = 8000;
    Ip4Addr serverAddr(ip, serverPort);

    ret = connect(sockFd, (struct sockaddr*)&serverAddr.getAddr(), sizeof(serverAddr.getAddr()));
    fatalif(ret < 0, "connect failed %d %s", errno, strerror(errno));

    int count = 0;
    char buf[200];
    memset(buf, '\0', 200);

    while(1){
        string msg = "msg: " + to_string(count++);
        int len = send(sockFd, (void*)msg.data(), msg.size(), 0);
        if(len < 0){
            info("send error, peer close fd");
            break;
        }
        
        info("send %d", len);

        memset(buf, '\0', 200);
        len = recv(sockFd, buf, 200, 0);
        
        if(len < 0){
            info("recv error, peer close fd");
            break;
        }
        
        info("recv %s, len %d", buf, len);
        sleep(2);
    }
    
    close(sockFd);
    info("client exit");
}


int main(){

    testClient();

    return 0;
}
