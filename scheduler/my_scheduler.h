//
// Created by DW on 2018/11/24.
//

#ifndef C_BASIC_MY_SCHEDULER_H
#define C_BASIC_MY_SCHEDULER_H

#include <map>
#include "noncopyable.h"
#include "my_threadpool.h"
#include "my_interruptSleep.h"

namespace xy{
    using Clock = std::chrono::system_clock;

    class Task{
        public:
            typedef std::function<void()> task_t;
            Task(task_t&& fun, bool interval=false):
                    f_(std::move(fun)), interval_(interval){}

            virtual Clock::time_point getNextTime() = 0;

            task_t getFun(){return f_;};

            bool bInterval(){return interval_;}

        private:
            task_t f_;
            bool interval_;
    };

    class TaskAt:public Task{
        public:
            TaskAt(task_t&& fun):Task(std::move(fun)){}

            virtual Clock::time_point getNextTime();
    };

    Clock::time_point TaskAt::getNextTime() {
        return Clock::time_point(Clock::duration(0));
    }

    class TaskEvery: public Task{
        public:
            TaskEvery(const Clock::duration dura, task_t&& fun):
                    Task(std::move(fun), true), dura_(dura){}

            virtual Clock::time_point getNextTime();

        private:
            Clock::duration dura_;
    };

    Clock::time_point TaskEvery::getNextTime(){
        return Clock::now() + dura_;
    }

    std::string timePoint2Str(const Clock::time_point timePoint){
        std::time_t t = Clock::to_time_t(timePoint);
        return std::ctime(&t);
    }

    class Scheduler: public noncopyable{
        public:
            Scheduler(unsigned int threadNums=5);
            ~Scheduler();

            template<typename _Callable, typename... _Args>
            void runAfter(const Clock::duration dura, _Callable&& f, _Args &&... args);

            template<typename _Callable, typename... _Args>
            void runEvery(const Clock::duration dura, _Callable&& f, _Args &&... args);

            template<typename _Callable, typename... _Args>
            void runAt(const Clock::time_point tPoint, _Callable&& f, _Args &&... args);

        private:
            ThreadPool threadPool_;
            InterruptSleep interruptSleep_;

    //            std::multimap<Clock::time_point, Task*> tasks_;
            std::multimap<Clock::time_point, std::shared_ptr<Task>> mapTasks_;
            std::mutex mtx_;
            bool bExit_;

            void addTask(const Clock::time_point timePoint, std::shared_ptr<Task> t);

            void execNearestAndUpdate();
    };

    Scheduler::Scheduler(unsigned int threadNum):threadPool_(threadNum), bExit_(false){
        auto lb_schedule = [this](){
            while(!bExit_){
                if(mapTasks_.empty()){
                    std::cout<<"schedule empty loop"<<std::endl;
                    interruptSleep_.sleep();
                }else{
                    auto nextNearest = mapTasks_.begin()->first;
                    std::cout<<"schedule not empty loop, sleep until to "<<timePoint2Str(nextNearest)<<std::endl;
                    interruptSleep_.sleep_until(nextNearest);
                }

                execNearestAndUpdate();
            }
        };

        threadPool_.addTask(lb_schedule);
        threadPool_.start();
    }

    Scheduler::~Scheduler(){
        bExit_ = true;
        interruptSleep_.interrupt();

        threadPool_.stop();
        threadPool_.join();
    }

    void Scheduler::execNearestAndUpdate(){
        std::lock_guard<std::mutex> lock(mtx_);
        auto endOfTaskToRun = mapTasks_.upper_bound(Clock::now());

    //        if(endOfTaskToRun != mapTasks_.begin())
    //            std::cout<<"execNearestAndUpdate, begin time:"<<timePoint2Str(mapTasks_.begin()->first)<<std::endl;
    //        else
    //            std::cout<<"execNearestAndUpdate, no need to do task, size is :"<<mapTasks_.size()<<std::endl;

        if(endOfTaskToRun != mapTasks_.begin()){
            decltype(mapTasks_) everyTask;

            for (auto it = mapTasks_.begin(); it != endOfTaskToRun; ++it) {
                auto& task = it->second;
    //                std::cout<<"execNearestAndUpdate, add task to thread pool: "<<timePoint2Str(it->first)<<std::endl;
                threadPool_.addTask(task->getFun());

                if(task->bInterval()){
                    everyTask.emplace(task->getNextTime(), task);
                }
            }

            // first remove old task, then add every task
            mapTasks_.erase(mapTasks_.begin(), endOfTaskToRun);

            for(auto& task: everyTask)
                mapTasks_.emplace(task.first, std::move(task.second));
        }

    }

    template<typename _Callable, typename... _Args>
    void Scheduler::runAfter(const Clock::duration dura, _Callable &&f, _Args &&... args) {
        runAt(Clock::now() + dura, std::forward<_Callable>(f), std::forward<_Args>(args)...);
    }

    template <typename _Callable, typename... _Args>
    void Scheduler::runAt(const Clock::time_point tPoint, _Callable &&f, _Args &&... args) {
        auto t = std::make_shared<TaskAt>(
                std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...));

        addTask(tPoint, t);
    }

    template<typename _Callable, typename... _Args>
    void Scheduler::runEvery(const Clock::duration dura, _Callable&& f, _Args &&... args){
        auto t = std::make_shared<TaskEvery>(
           dura, std::bind(std::forward<_Callable>(f), std::forward<_Args>(args)...)
        );

        auto nextTime = t->getNextTime();
        addTask(nextTime, t);
    }

    void Scheduler::addTask(const Clock::time_point timePoint, std::shared_ptr<Task> t){
        std::lock_guard<std::mutex> lock(mtx_);

        mapTasks_.emplace(timePoint, std::move(t));
        interruptSleep_.interrupt();
    //        std::cout<<"add task time point: "<<timePoint2Str(timePoint)<<", task size is: "<<mapTasks_.size()<<std::endl;
    }

}

#endif //C_BASIC_MY_SCHEDULER_H
