//
// Created by root on 11/27/18.
//

#include <iostream>
#include <unordered_map>
#include "handy/handy.h"

using namespace std;
using namespace handy;


void test_single_server(){
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(serverFd < 0, "socket failed %d %s", errno, strerror(errno));

    string ip = "127.0.0.1";
    uint16_t port = 8000;
    Ip4Addr  localAddr(ip, port);

    int ret = bind(serverFd, (struct sockaddr*)&localAddr.getAddr(),
                   sizeof(localAddr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    ret = listen(serverFd, 100);
    fatalif(ret<0, "listen socket failed, %d, %s", errno, strerror(errno));

    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cFd = accept(serverFd, (struct sockaddr *) &raddr, &rsz);

    fatalif(cFd<0, "accept failed");

    Ip4Addr tmp(raddr);
    info("server fd:%d, client fd: %d, accept a connection from %s", serverFd, cFd, tmp.toString().c_str());

    fd_set fds;
    timeval tv;
    const int BUF_SIZE = 200;

    while(1) {
        // 把可读文件描述符的集合清空
        FD_ZERO(&fds);
        // 把标准输入的文件描述符加入到集合中
        FD_SET(0, &fds);
        // 把当前连接的文件描述符加入到集合中
        FD_SET(cFd, &fds);

        tv.tv_sec = 4;
        tv.tv_usec = 500;

        ret = select(cFd+1, &fds, NULL, NULL, &tv);

        if(ret < 0){
            warn("select error, server exit");
            break;
        } else if(0 == ret){
            info("no input from server, client no message, so keep waiting.....");
        } else{

            if(FD_ISSET(cFd, &fds)){
                char recvBuf[BUF_SIZE];
                int len = recv(cFd, recvBuf, sizeof(recvBuf), 0);
                info("recv len: %d, msg: %s", len, recvBuf);
            }

            if(FD_ISSET(0, &fds)){
                char sendBuf[BUF_SIZE];
                cin>>sendBuf;

                int len = send(cFd, sendBuf, strlen(sendBuf), 0);
                info("send %d bytes to server", len);

                memset(sendBuf, 0, sizeof(sendBuf));
            }
        }

    }

    close(serverFd);
    close(cFd);
}

class ClientInfo{
    public:
        ClientInfo(const string& ip, uint16_t port, int fd):
                addr_(ip, port), fd_(fd){
            cout<<"ClientInfo ctor"<<endl;
        }

        ClientInfo(const Ip4Addr& addr, int fd):
                addr_(addr), fd_(fd){
            cout<<"ClientInfo ctor"<<endl;
        }

        ClientInfo(const ClientInfo& ip):addr_(ip.addr_), fd_(ip.fd_){
            cout<<"ClientInfo copy ctor"<<endl;
        }

        string getClientInfo()const{
            return addr_.toString() + "_" + to_string(fd_);
        }

    private:
        Ip4Addr addr_;
        int fd_;
};

std::mutex gMtx;
int svrFd;
unordered_map<int, ClientInfo> mFdClient;
set<int> sFd;

void infoClients(){
    std::lock_guard<std::mutex> lock(gMtx);

    string msg = "\n ---------- select timeout, current client info ----------\n";

    for (auto& item: mFdClient)
        msg = msg + item.second.getClientInfo() + " ";

    msg += "\n----------------------------------------\n";
    info("%s", msg.c_str());

}

void handAccept(int svrFd){
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);

    int newCFd = accept(svrFd, (struct sockaddr *) &raddr, &rsz);
    if(newCFd<0){
        error("accept failed");
        return;
    }


    Ip4Addr tmp(raddr);
    info("server fd:%d, client fd: %d, accept a connection from %s", svrFd, newCFd, tmp.toString().c_str());

    {
        std::lock_guard<std::mutex> lock(gMtx);
        sFd.insert(newCFd);
//        mFdClient.emplace(newCFd, {tmp, newCFd});
        mFdClient.emplace(newCFd, ClientInfo(tmp, newCFd));
    }
}

void boardcast(const string& msg){
    for(auto& item:mFdClient){
        int len = send(item.first, msg.data(), msg.size(), 0);

        if(len <=0 ){
            error("boardcast, len:%d, info:%s, error: errno: %d, %s, server exit", len,
                  mFdClient.find(item.first)->second.getClientInfo().c_str(), errno, strerror(errno));
        }
    }

}

void handRead(int acFd){
    char buf[100];
    memset(buf, 0, sizeof buf);

    int len = recv(acFd, buf, sizeof buf, 0);

    if(len <= 0){
        if(len < 0){
            error("recv error: errno: %d, %s, server exit", errno, strerror(errno));
            return;
        }
        else
            info("client: %s, close", mFdClient.find(acFd)->second.getClientInfo().c_str());

        const char *welcome = "%s: good I leave out";
        string msg = util::format(welcome, mFdClient.find(acFd)->second.getClientInfo().c_str());

        close(acFd);
        mFdClient.erase(acFd);
        sFd.erase(acFd);

        boardcast(msg);

    } else if(len > 0){
        string msg(buf, len);
//        msg = util::format("%s: %s", mFdClient.find(acFd)->second.getClientInfo().c_str(), msg);
        msg = mFdClient.find(acFd)->second.getClientInfo() + ": " + msg;
        boardcast(msg);
    }

}

void processFd(int acFd){
    if(acFd == svrFd){
        handAccept(svrFd);
    } else{
        handRead(acFd);
    }
}


void test_multi_server(){
    svrFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(svrFd<0, "sock failed %d, %s", errno, strerror(errno));
    sFd.insert(svrFd);

    net::setNonBlock(svrFd);
    net::setReuseAddr(svrFd);

    string ip = "172.25.53.26";
    uint16_t port = 8888;
    Ip4Addr  localAddr(ip, port);

    int ret = bind(svrFd, (struct sockaddr*)&localAddr.getAddr(),
                   sizeof(localAddr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    ret = listen(svrFd, 20000);
    fatalif(ret<0, "listen socket failed, %d, %s", errno, strerror(errno));

    ThreadPool tp(1);

    fd_set fds;
    timeval tv;

    while(1){
        FD_ZERO(&fds);

        for (int fd: sFd) {
            FD_SET(fd, &fds);
        }

        tv.tv_sec = 10;

        int num = select((*sFd.rbegin())+1, &fds, NULL, NULL, &tv);

        if(num < 0){
            error("select error, server exit");
            break;
        } else if(num == 0){
            infoClients();
        } else{
            for(int fd:sFd){
                if(FD_ISSET(fd, &fds)){
                    tp.addTask(std::bind(processFd, fd));
//                    processFd(fd);
                }

            }
        }
    }
}

void test_thread_pool(){

}

int main(){
//    test_single_server();

    test_multi_server();



    return 0;
}