#include "InetAddress.h"
#include "Logger.h"

#include <strings.h>
#include <string.h>

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    //addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    // 使用 inet_pton 替代老旧且不安全的 inet_addr
    int ret = ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
    if (ret != 1) {
        // 如果转换失败，打印具体的错误 IP，方便排查
        LOG_ERROR("Invalid IP address: %s", ip.c_str());
        return; 
    }
}

std::string InetAddress::toIp() const
{
    // addr_
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

std::string InetAddress::toIpPort() const
{
    // ip:port
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

// #include <iostream>
// int main()
// {
//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;

//     return 0;
// }