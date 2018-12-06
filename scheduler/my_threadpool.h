//
// Created by DW on 2018/11/22.
//

#ifndef C_BASIC_MY_THREADPOOL_H
#define C_BASIC_MY_THREADPOOL_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <list>
#include <iostream>
#include <functional>

#include "noncopyable.h"

namespace xy{

    template<typename T>
    class SafeQueue: public noncopyable{
        static const int WAIT_INFINITE = std::numeric_limits<int>::max();

        public:
            SafeQueue():stop_(false){}

            bool pop(T& value){
                std::lock_guard<std::mutex> lock(mtx_);
                if(queue_.empty())
                    return false;

                value = queue_.front();
                queue_.pop_front();
                return true;
            }

            T pop_wait(int waitMs=1000){// wait 1s by default
                std::unique_lock<std::mutex> lock(mtx_);
                wait_ready(lock);

                if(queue_.empty()) return NULL;

                T value = queue_.front();
                queue_.pop_front();
                return value;
            }

            /*
             * 疑问这里 T&&参数结合move问题
             */
            //void push(const T& value){
            void push(T&& value){
                std::lock_guard<std::mutex> lock(mtx_);
                queue_.push_back(std::move(value));

                cv_.notify_one();
            }

            void push(const T& value){
                std::lock_guard<std::mutex> lock(mtx_);
                queue_.push_back(value);
                cv_.notify_one();
            }

            bool empty(){
                std::lock_guard<std::mutex> lock(mtx_);
                return queue_.empty();
            }

            void stop(){
                stop_ = true;
                cv_.notify_all();
            }

        private:
            void wait_ready(std::unique_lock<std::mutex>& lock, int waitMs=WAIT_INFINITE);

        private:
            std::mutex mtx_;
            std::list<T> queue_;  // list可能更合适
            // std::queue<T> queue_;
            std::condition_variable cv_;
            bool stop_;
    };

    template<typename T>
    void SafeQueue<T>::wait_ready(std::unique_lock<std::mutex>& lock, int waitMs){
        auto lb_cond = [this](){ return !queue_.empty() || stop_; };

        if(waitMs == WAIT_INFINITE){
            cv_.wait(lock, lb_cond);
        }else{
            auto t = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitMs);
            cv_.wait_until(lock, t, lb_cond);
        }
    }

    typedef std::function<void()> task_t;
    typedef SafeQueue<task_t> Tasks;

    class ThreadPool: public noncopyable{
        public:
            ThreadPool(uint32_t nums=5, bool start=false);
            ~ThreadPool();

            void start();

            void addTask(task_t&& task);
            void addTask(const task_t& task);

            void join();
            void detach();
            void stop();
            size_t getThreadNum(){return threadNums_;}

        private:
            Tasks tasks_;

            std::vector<std::thread> threads_;
            size_t threadNums_;
            bool stop_;
    };

    ThreadPool::ThreadPool(uint32_t nums, bool start):threadNums_(nums), stop_(false){
        if(start)
            this->start();
    }

    ThreadPool::~ThreadPool(){
        stop();
    }

    void ThreadPool::start(){
        auto lb_thread_fun = [this](){
            while (!stop_){
                task_t task;
//                tasks_.pop(task);
                task = tasks_.pop_wait();
                if(NULL == task){
                    std::cout<<"get null task"<<std::endl;
                } else{
                    task();
                }
            }
        };

        for (int i = 0; i < threadNums_; ++i) {
            threads_.push_back(std::thread(lb_thread_fun));
        }
    }

    void ThreadPool::addTask(task_t&& task){
//        tasks_.push(task); // compile error
        tasks_.push(std::move(task));
    }

    void ThreadPool::addTask(const task_t& task){
        tasks_.push(task);
    }

    void ThreadPool::join(){
        for (auto& th:threads_) {
            th.join();
        }
    }

    void ThreadPool::detach(){
        for(auto& th:threads_){
            th.detach();
        }
    }

    void ThreadPool::stop(){
        stop_ = true;
        tasks_.stop();
    }

}

#endif //C_BASIC_MY_THREADPOOL_H
