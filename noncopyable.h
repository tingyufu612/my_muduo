#pragma once
//noncopyable 被继承之后派生类对象可以正常构造和析构，但无法拷贝构造和赋值
class noncopyable
{
    public:
    noncopyable(const noncopyable&)=delete;
    noncopyable& operator=(const noncopyable&) =delete;
    protected:
    noncopyable()=default;
    ~noncopyable()=default;
};
