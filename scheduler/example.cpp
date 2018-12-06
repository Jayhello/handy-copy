//
// Created by DW on 2018/11/22.
//
#include <iostream>

#include "Scheduler.h"

using namespace std;

void message(const std::string &s) {
    std::cout << s << std::endl;
}

/*
 *  Source example
 */
void example_1(){
    // number of tasks that can run simultaneously
    // Note: not the number of tasks that can be added,
    //       but number of tasks that can be run in parallel
    unsigned int max_n_threads = 12;

    // Make a new scheduling object.
    // Note: s cannot be moved or copied
    Bosma::Scheduler s(max_n_threads);

    // every second call message("every second")
    s.every(std::chrono::seconds(1), message, "every second");

    // in one minute
    s.in(std::chrono::minutes(1), []() { std::cout << "in one minute" << std::endl; });

    // run lambda, then wait a second, run lambda, and so on
    // different from every in that multiple instances of the function will never be run concurrently
    s.interval(std::chrono::seconds(1), []() {
        std::cout << "right away, then once every 6s" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    });

    // https://en.wikipedia.org/wiki/Cron
    s.cron("* * * * *", []() { std::cout << "top of every minute" << std::endl; });

    // Time formats supported:
    // %Y/%m/%d %H:%M:%S, %Y-%m-%d %H:%M:%S, %H:%M:%S
    // With only a time given, it will run tomorrow if that time has already passed.
    // But with a date given, it will run immediately if that time has already passed.
    s.at("2018-011-23 12:31:15", []() { std::cout << "at a specific time." << std::endl; });

    s.cron("5 0 * * *", []() { std::cout << "every day 5 minutes after midnight" << std::endl; });

    // destructor of Bosma::Scheduler will cancel all schedules but finish any tasks currently running
    std::this_thread::sleep_for(std::chrono::minutes(10));
}

void heartBeat(uint32_t interval, const string& msg){
    cout<<"now sending heart beat: "<<msg<<endl;
}

/*
 * My own example.
 */
void my_example(){
    // thread num
    unsigned int threadNum = 10;

    // scheduler
    Bosma::Scheduler scheduler(threadNum);

    uint32_t heartBeatSec = 5;
    scheduler.every(std::chrono::seconds(5), heartBeat, heartBeatSec, "this is client, every 5");

    scheduler.interval(std::chrono::seconds(3), heartBeat, 3, "this is client, every 3");

    scheduler.in(std::chrono::seconds(6), heartBeat, 0, "goodbye, ending.....");

    std::this_thread::sleep_for(std::chrono::minutes(1));
}

int main() {
    my_example();

    return 0;
}
