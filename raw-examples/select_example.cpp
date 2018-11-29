//
// Created by root on 11/27/18.
//

#include <sys/types.h>
#include <cstring>
#include <cstdio>

#include "handy/handy.h"
#include <iostream>

using namespace std;
using namespace handy;

void simple_test(){
    const int buf_size = 200;
    char buf[buf_size];
    memset(buf, '\0', sizeof(buf));

    fd_set fds;
    FD_ZERO(&fds);
    // 把标准输入的文件描述符 0 加入到集合中 (cin 0, cout 1, cerr 2)
    FD_SET(0, &fds);

    timeval tv;   // timeout 5.5 second
    tv.tv_sec = 5;
    tv.tv_usec = 500;

    int ret = select(1, &fds, nullptr, nullptr, &tv);

    if(ret < 0)
        perror("select error");
    else if(ret == 0)
        cout<<"select timeout"<<endl;
    else{
        scanf("%s", buf);
        cout<<"buf is: "<<buf;
    }

    cout<<"----- ending -----"<<endl;
}

void see_errno_reason(){
    for (int i = 0; i < 134; ++i) {
        cout<<"errno: "<<i<<", reason: "<<strerror(i)<<endl;
    }
}


void chat_client(){
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(sockFd<0, "socket failed %d %s", errno, strerror(errno));

    uint16_t serverPort = 8888;
    string ip = "172.25.53.26";
    Ip4Addr serverAddr(ip, serverPort);

    int ret = connect(sockFd, (struct sockaddr*)&serverAddr.getAddr(), sizeof(serverAddr.getAddr()));
    fatalif(ret < 0, "connect failed %d %s", errno, strerror(errno));

    info("connect to server success, sock fd: %d", sockFd);

    fd_set fds;
    timeval tv;
    const int BUF_SIZE = 200;


    while(1){
        FD_ZERO(&fds);
        FD_SET(0, &fds);

        tv.tv_sec = 3;
        tv.tv_usec = 500;

        // 把标准输入的文件描述符加入到集合中
        FD_SET(0, &fds);
        // 把当前连接的文件描述符加入到集合中
        FD_SET(sockFd, &fds);

        ret = select(sockFd + 1, &fds, NULL, NULL, &tv);

        if(ret < 0){
            warn("select error, client exit");
            break;
        } else if(0 == ret){
            info("no input from client, server no message, so keep waiting.....");
        } else{

            if(FD_ISSET(sockFd, &fds)){
                char recvBuf[BUF_SIZE];
                memset(recvBuf, 0, sizeof(recvBuf));

                int len = recv(sockFd, recvBuf, sizeof(recvBuf), 0);
                if(0 == len){ // means server side close
                    info("recv error %d, %s...", errno, strerror(errno));
                    // recv error 0, Success...
                    break;
                }

                info("recv len: %d, msg: %s", len, recvBuf);
            }

            if(FD_ISSET(0, &fds)){
                char sendBuf[BUF_SIZE];
                cin>>sendBuf;

                int len = send(sockFd, sendBuf, strlen(sendBuf), 0);
                info("send %d bytes to server", len);

                memset(sendBuf, 0, sizeof(sendBuf));
            }
        }

    }

    close(sockFd);
    info("client exit, bye......");
}


int main(){

//    simple_test();

    chat_client();

//    see_errno_reason();

    return 0;
}