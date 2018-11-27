//
// Created by root on 11/27/18.
//

#include <sys/types.h>
#include <cstring>
#include <cstdio>

#include <iostream>
using namespace std;


void simple_test(){
    const int buf_size = 200;
    char buf[buf_size];
    memset(buf, '\0', sizeof(buf));

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);

    timeval tv;
    tv.tv_sec = 3;
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


int main(){

    simple_test();

    return 0;

}