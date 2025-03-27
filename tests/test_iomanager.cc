/**
 * @file test_iomanager.cc 
 * @brief IO协程调度器测试
 * @details 通过IO协程调度器实现一个简单的TCP客户端，这个客户端会不停地判断是否可读，并把读到的消息打印出来
 *          当服务器关闭连接时客户端也退出
 * @version 0.1
 * @date 2021-06-16
 */
#include "awcotn/awcotn.h"
#include "awcotn/iomanager.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>


int sockfd;
void watch_io_read();
awcotn::Logger::ptr g_logger = AWCOTN_LOG_ROOT();
// 写事件回调，只执行一次，用于判断非阻塞套接字connect成功

// 读事件回调，每次读取之后如果套接字未关闭，需要重新添加
void do_io_read() {
    char buf[1024] = {0};
    int readlen = 0;
    readlen = read(sockfd, buf, sizeof(buf));
    if(readlen > 0) {
        buf[readlen] = '\0';
    } else if(readlen == 0) {
        close(sockfd);
        return;
    } else {
        close(sockfd);
        return;
    }
    // read之后重新添加读事件回调，这里不能直接调用addEvent，因为在当前位置fd的读事件上下文还是有效的，直接调用addEvent相当于重复添加相同事件
    awcotn::IOManager::GetThis()->schedule(watch_io_read);
}

void watch_io_read() {
    awcotn::IOManager::GetThis()->addEvent(sockfd, awcotn::IOManager::READ, do_io_read);
}

void test_io() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(1234);
    inet_pton(AF_INET, "10.10.19.159", &servaddr.sin_addr.s_addr);

    int rt = connect(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr));
    if(rt != 0) {
        if(errno == EINPROGRESS) {
            AWCOTN_LOG_INFO(g_logger) << "EINPROGRESS";
            awcotn::IOManager::GetThis()->addEvent(sockfd, awcotn::IOManager::READ, do_io_read);
        } else {
            AWCOTN_LOG_ERROR(g_logger) << "connect error, errno:" << errno << ", errstr:" << strerror(errno);
        }
    } else {
    }
}

void test_iomanager() {
    awcotn::IOManager iom;
    // awcotn::IOManager iom(10); // 演示多线程下IO协程在不同线程之间切换
    iom.schedule(test_io);
}

int main(int argc, char** argv) {
    awcotn::IOManager iom;
    iom.start();  // 显式调用start()，将m_stopping设为false
    iom.schedule(test_io);
    // ...等待完成...
    return 0;
}