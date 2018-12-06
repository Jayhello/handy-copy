//
// Created by DW on 2018/11/25.
//

#ifndef C_BASIC_NONCOPYABLE_H
#define C_BASIC_NONCOPYABLE_H
namespace xy{

    class noncopyable{
    public:
        noncopyable() = default;
        ~noncopyable() = default;
    private:
        noncopyable(const noncopyable&) = delete;
        noncopyable& operator=(const noncopyable&) = delete;
    };

}
#endif //C_BASIC_NONCOPYABLE_H
