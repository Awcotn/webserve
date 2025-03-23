#ifndef __WEB__LOG_H__
#define __WEB__LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <map>
#include "singleton.h"
#include "util.h"
#include "thread.h"

#define AWCOTN_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        awcotn::LogEventWrap(awcotn::LogEvent::ptr(new awcotn::LogEvent(logger, level, __FILE__, __LINE__, 0, awcotn::GetThreadId(),\
            awcotn::GetFiberId(), time(0), awcotn::Thread::GetName()))).getSS()
        
#define AWCOTN_LOG_DEBUG(logger) AWCOTN_LOG_LEVEL(logger, awcotn::LogLevel::DEBUG)
#define AWCOTN_LOG_INFO(logger) AWCOTN_LOG_LEVEL(logger, awcotn::LogLevel::INFO)
#define AWCOTN_LOG_WARN(logger) AWCOTN_LOG_LEVEL(logger, awcotn::LogLevel::WARN)
#define AWCOTN_LOG_ERROR(logger) AWCOTN_LOG_LEVEL(logger, awcotn::LogLevel::ERROR)
#define AWCOTN_LOG_FATAL(logger) AWCOTN_LOG_LEVEL(logger, awcotn::LogLevel::FATAL)

#define AWCOTN_LOG_ROOT() awcotn::LoggerMgr::GetInstance()->getRoot()
#define AWCOTN_LOG_NAME(name) awcotn::LoggerMgr::GetInstance()->getLogger(name)

namespace awcotn {

class Logger;
class LoggerManager;

//日志级别
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level);
    
    static LogLevel::Level FromString(const std::string& str);
};


//日志事件  
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
        , const char* file, int32_t line, uint32_t elapse
        , uint32_t threadId, uint32_t fiberId, uint64_t time
        , const std::string thread_name);

    

    const char* getFile() const { return m_files; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    int32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getThreadName() const { return m_threadName; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    
    std::stringstream& getSS() { return m_ss; }
    void format (const char* fmt, ...);
    void format (const char* fmt, va_list al);
private:
    const char* m_files = nullptr;  //文件名
    int32_t m_line = 0;             //行号
    uint32_t m_elapse = 0;          //程序启动开始到现在的毫秒数    
    int32_t m_threadId = 0;         //线程id 
    uint32_t m_fiberId = 0;         //协程id
    uint64_t m_time = 0;            //时间戳
    std::string m_threadName;
    std::stringstream m_ss;    
    
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();

    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;
};

//日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    //%t    %thread_id %m%n
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
public: 
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();

    bool isError () const { return m_error; }

    const std::string getPattern() const { return m_pattern; }
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};

//日志输出地
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;
    virtual ~LogAppender() {};

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter () ;

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    bool m_hasFormatter = false;
    Mutex m_mutex;
    LogFormatter::ptr m_formatter;
};

//日志器
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");   
    void log(LogLevel::Level level, const LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }

    void setFormatter(LogFormatter::ptr val) ;
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter () ;

    const std::string& getName() const { return m_name; }

    std::string toYamlString();
private:
    std::string m_name;                     //日志名称
    LogLevel::Level m_level;                //日志级别
    Mutex m_mutex;
    std::list<LogAppender::ptr> m_appenders;//Appender集合
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
};

//输出到控制台
class StdoutLogAppender : public LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event) override;
    std::string toYamlString() override;
private:
};

//输出到文件
class FileLogAppender : public LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event) override;
    std::string toYamlString() override;
    //重新打开文件，成功true
    bool reopen();
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
};

class LoggerManager {
public:
    typedef Spinlock MutexType;
    LoggerManager();
    Logger::ptr getLogger(const std::string& name);

    void init();
    Logger::ptr getRoot() const { return m_root; }

    std::string toYamlString();
private:
    Mutex m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef awcotn::Singleton<LoggerManager> LoggerMgr;

};

#endif