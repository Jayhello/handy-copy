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
    net::setReuseAddr(serverFd);

//    string ip = "172.25.53.26";
    string ip = "127.0.0.1";
    uint16_t port = 8888;
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

    int idx = 0;

    while(1) {
        // 把可读文件描述符的集合清空
        FD_ZERO(&fds);
        // 把标准输入的文件描述符加入到集合中
        FD_SET(0, &fds);
        // 把当前连接的文件描述符加入到集合中
        FD_SET(cFd, &fds);

        tv.tv_sec = 4;
        tv.tv_usec = 1000;

        ret = select(cFd+1, &fds, NULL, NULL, &tv);

        if(ret < 0){
            warn("select error, server exit");
            break;
        } else if(0 == ret){
            info("no input from server, client no message, so keep waiting.....");
        } else{

            if(0 == idx++ % 2){
                info("idx: %d even, so continue to next", idx);
                continue;
            }

            if(FD_ISSET(cFd, &fds)){
                char recvBuf[BUF_SIZE];
                memset(recvBuf, 0, sizeof(recvBuf));
                int len = recv(cFd, recvBuf, sizeof(recvBuf), 0);

                if(len <= 0){
                    info("recv len: %d, errno: %d, msg: %s", len, errno, strerror(errno));

                    len = send(cFd, recvBuf, strlen(recvBuf), 0);
                    info("send %d bytes to server, , errno: %d, msg: %s", len, errno, strerror(errno));
                    sleep(1);
                }

                info("recv len: %d, msg: %s", len, recvBuf);
            }

            if(FD_ISSET(0, &fds)){
                char sendBuf[BUF_SIZE];
                memset(sendBuf, 0, sizeof(sendBuf));
                cin>>sendBuf;

                int len = send(cFd, sendBuf, strlen(sendBuf), 0);
                info("send %d bytes to server", len);

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

void addClientFd(int fd, const Ip4Addr& addr){
    std::lock_guard<std::mutex> lock(gMtx);
    sFd.insert(fd);
    mFdClient.emplace(fd, ClientInfo(addr, fd));
}

void removeClientFd(int fd){
    std::lock_guard<std::mutex> lock(gMtx);
    sFd.erase(fd);
    mFdClient.erase(fd);
    close(fd);
}

void handAccept(int fd, const sockaddr_in& addr){
    if(fd<0){
        error("accept failed");
        return;
    }

    Ip4Addr tmp(addr);
    info("server fd:%d, client fd: %d, accept a connection from %s", svrFd, fd, tmp.toString().c_str());

    addClientFd(fd, tmp);
}

void boardcast(const string& msg){
    std::lock_guard<std::mutex> lock(gMtx);
    for(auto& item:mFdClient){
        int len = send(item.first, msg.data(), msg.size(), 0);
        info("sending %s, len: %d", msg.data(), msg.size());

        if(len <=0 ){
            error("boardcast, len:%d, info:%s, error: errno: %d, %s, server exit", len,
                  mFdClient.find(item.first)->second.getClientInfo().c_str(), errno, strerror(errno));
        }
    }

}

void handRead(int acFd, const string& buf, int len, const string& clientInfo){
    if(len >= 0){

        if(0 == len){
            const char *welcome = "%s: good I leave out";
            string msg = util::format(welcome, clientInfo.c_str());
            boardcast(msg);
        }else{
            string msg = clientInfo + ": " + buf;
            boardcast(msg);
        }
    } else{
        error("recv error: errno: %d, %s", errno, strerror(errno));
    }
}

//void processFd(int acFd){
//    if(acFd == svrFd){
//        handAccept(svrFd);
//    } else{
//        handRead(acFd);
//    }
//}


void test_multi_server(){
    svrFd = socket(AF_INET, SOCK_STREAM, 0);
    fatalif(svrFd<0, "sock failed %d, %s", errno, strerror(errno));
    sFd.insert(svrFd);

//    net::setNonBlock(svrFd);
    net::setReuseAddr(svrFd);

//    string ip = "172.25.53.26";
    string ip = "127.0.0.1";
    uint16_t port = 8888;
    Ip4Addr  localAddr(ip, port);

    int ret = bind(svrFd, (struct sockaddr*)&localAddr.getAddr(),
                   sizeof(localAddr.getAddr()));
    fatalif(ret < 0, "bind socket failed %d %s", errno, strerror(errno));

    ret = listen(svrFd, 20000);
    fatalif(ret<0, "listen socket failed, %d, %s", errno, strerror(errno));
    info("start server success %s", localAddr.toString().c_str());

    ThreadPool tp(5);

    fd_set fds;
    timeval tv;

    while(1){
        FD_ZERO(&fds);

        {
            std::lock_guard<std::mutex> lock(gMtx);
            for (int fd: sFd) {
                FD_SET(fd, &fds);
            }
        }


        tv.tv_sec = 100;

        int num = select((*sFd.rbegin())+1, &fds, NULL, NULL, &tv);

        if(num < 0){
            error("select error, server exit");
            break;
        } else if(num == 0){
            infoClients();
        } else{
            for(int fd:sFd){
                if(FD_ISSET(fd, &fds)){
                     if(fd == svrFd){
                         struct sockaddr_in raddr;
                         socklen_t rsz = sizeof(raddr);
                         int newCFd = accept(svrFd, (struct sockaddr *) &raddr, &rsz);
                         info("select return %d events, fd: %d, new accept newFd: %d", num, fd, newCFd);
//                         handAccept(newCFd, raddr);
//                         tp.addTask(std::bind(handAccept, newCFd, std::ref(raddr)));
                         handAccept(newCFd, raddr);
                     } else{
                         char buf[100];
                         memset(buf, 0, sizeof buf);
                         int len = recv(fd, buf, sizeof buf, 0);

                         string clientInfo;
                         {
                             std::lock_guard<std::mutex> lock(gMtx);
                             auto it = mFdClient.find(fd);
                             if(it != mFdClient.end())
                                 clientInfo = it->second.getClientInfo();
                             else
                                 clientInfo = "error client";
                         }

                         if(0 == len)removeClientFd(fd);

                         info("select return %d events, fd: %d, new data, len: %d", num, fd, len);
//                         handRead(fd, buf, len);
                         tp.addTask(std::bind(handRead, fd, string(buf), len, std::ref(clientInfo)));
                     }

                }
            }
        }
    }
}

list<ClientInfo> lstCi;
std::mutex mtx;

void test_insert_ci(const ClientInfo& ci){
    std::lock_guard<std::mutex> lock(mtx);
    lstCi.push_back(ci);
    info("lst size: %d", lstCi.size());
}

void test_thread_pool(){

    ThreadPool tp(5);

    for (int i = 0; i < 1000; ++i) {
        info("loop %d", i);
        ClientInfo ci(to_string(i), i, i);

        tp.addTask(std::bind(test_insert_ci, ci));

        info("now i sleep 10 seconds");
        sleep(10);
    }
}

void test_thread_pool_arr(){

    auto lb_print = [](const char* str){
        info("print msg: %s, and i will sleep 2s", str);
        sleep(2);
    };

    ThreadPool tp(3);

    {
        char arr[100];
        memset(arr, 1, sizeof arr);
        tp.addTask(std::bind(lb_print, arr));
        tp.addTask(std::bind(lb_print, arr));
    }

    tp.exit();
    tp.join();
    info("ending....");
}


int main(){
    test_thread_pool_arr();

//    test_single_server();

//    test_multi_server();

//    test_thread_pool();



    return 0;
}