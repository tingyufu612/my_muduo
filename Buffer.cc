#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

/**
 * 从fd上读取数据  Poller工作在LT模式
 * Buffer缓冲区是有大小的 但是从fd上读数据的时候，却不知道tcp数据最终的大小
 */ 

ssize_t Buffer::readFd(int fd ,int* saveErrno)
{
    char extrabuf[65536]={0};//栈上内存空间
    struct iovec vec[2];
    const size_t writable =writableBytes();
    vec[0].iov_base=begin()+writerIndex_;
    vec[0].iov_len=writable;

    vec[1].iov_base=extrabuf;
    vec[1].iov_len=sizeof extrabuf;

    const int iovcnt=(writable<sizeof extrabuf)?2:1;//不够64k选择2
    const ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;

    }
    else if(n<=writable)
    {
        writerIndex_ +=n;

    }
    else//extrabuf里也写入数据
    {
        writerIndex_=buffer_.size();
        append(extrabuf,n-writable);
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if (n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}