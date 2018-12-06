//
// Created by DW on 2018/11/22.
//

#include "my_scheduler.h"

#include <iostream>
#include <ratio>

#include <sys/epoll.h>

using namespace std;
using std::cout;
using std::endl;

auto lb_dummy_dw = [](const std::string& url){
    cout<<"start downloading: "<<url<<endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    cout<<"downloading success !!!!"<<url<<endl;
};

auto lb_dummy_sql = [](int id, const std::string& name){
    cout<<"start select from db, id:" << id << ", name: "<<name<<endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    cout<<"select db success !!!!"<<endl;
};

void test_thread_pool(){
    cout<<"create thread pool with 5 thread"<<endl;
    xy::ThreadPool tp(5);

    cout<<"add 3 * 2 task to thread pool"<<endl;
    for (int i = 0; i < 3; ++i) {
        tp.addTask(std::bind(lb_dummy_dw, "ww.xxx.com"));
        tp.addTask(std::bind(lb_dummy_sql, i, "xy" + std::to_string(i)));
    }

    cout<<"start thread pool"<<endl;
    tp.start();

//    tp.join();
    tp.detach();

    this_thread::sleep_for(chrono::seconds(50));
}

void test_task(){
    xy::task_t task;
    try{
        task();
    }catch (std::exception& e){
        cout<<"running task, with exception..."<<e.what()<<endl;
        return;
    }

    cout<<"ending task, without error"<<endl;
}

void test_schedule(){
    cout<<"create new scheduler"<<endl;
    xy::Scheduler scheduler;

    cout<<"now: "<<xy::timePoint2Str(xy::Clock::now())<<endl;

    cout<<"add new task runAfter to scheduler"<<endl;
    scheduler.runAfter(std::chrono::seconds(2), lb_dummy_dw, "ww.xxx.com");

    cout<<"add new task runEvery to scheduler"<<endl;
    scheduler.runEvery(std::chrono::seconds(2), lb_dummy_sql, 2, "xy2");

    cout<<"add new task runAt to scheduler"<<endl;
    scheduler.runAt(std::chrono::system_clock::now() + std::chrono::seconds(10), lb_dummy_sql, 11111111, "nownownownownow");

    //    cout<<"add new task runEvery to scheduler"<<endl;
    //    scheduler.runEvery(std::chrono::milliseconds(100), lb_dummy_sql, 2, "xy2");

    //    cout<<"add new task runEvery to scheduler"<<endl;
    //    scheduler.runEvery(std::chrono::milliseconds(100), lb_dummy_dw, "ww.xxx.com");

    cout<<"main thread sleep 30 seconds"<<endl;
    this_thread::sleep_for(chrono::seconds(30));
    cout<<"------------ main thread sleep 3 seconds ending ------------"<<endl;
}



int main(){
    test_schedule();
//    test_thread_pool();

//    test_task();




    return 0;
}