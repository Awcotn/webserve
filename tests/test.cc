#include <iostream>
#include "awcotn/log.h"
#include "awcotn/util.h"

int main(int argc, char** argv) {
    awcotn::Logger::ptr logger(new awcotn::Logger);
    logger->addAppender(awcotn::LogAppender::ptr(new awcotn::StdoutLogAppender));

    awcotn::FileLogAppender::ptr file_appender(new awcotn::FileLogAppender("./log.txt"));
    awcotn::LogFormatter::ptr fmt(new awcotn::LogFormatter("%d%T%t%m%n"));
    file_appender->setFormatter(fmt);

    file_appender->setLevel(awcotn::LogLevel::DEBUG);
    //std::cout << file_appender << std::endl;
    logger->addAppender(file_appender);

    //awcotn::LogEvent::ptr event(new awcotn::LogEvent(__FILE__, __LINE__, 0, awcotn::GetThreadId(), awcotn::GetFiberId(), time(0)));
    //event->getSS() << "hello sylar log";
    //logger->log(awcotn::LogLevel::DEBUG, event);

    AWCOTN_LOG_INFO(logger) << "test macro";

    AWCOTN_LOG_ERROR(logger) << "test macro error";

    auto l = awcotn::LoggerMgr::GetInstance()->getLogger("xx");
    AWCOTN_LOG_INFO(l) << "xxx";


    return 0;
}