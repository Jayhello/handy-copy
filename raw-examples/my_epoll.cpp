//
// Created by root on 18-11-21.
//

#include <sys/socket.h>

#include <vector>
#include <iostream>

using namespace std;

/*
 * Note: Simple test for max socket num.
 */
void testMaxSockFd(){
    vector<int> vecFd(40);

    for (int i = 0; i < vecFd.size(); ++i) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        if(fd < 0){
            cerr<<i<<", get error fd: "<<fd<<endl;
            return;
            // 4093, get error fd: -1
        }
        cerr<<i<<", get fd: "<<fd<<endl;
        vecFd[i] = fd;
    }
}





int main(){

    return 0;
}