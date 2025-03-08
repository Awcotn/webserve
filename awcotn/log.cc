#include "log.h"
#include <iostream>
#include <map>
#include <functional>
namespace awcotn
{

const char *LogLevel::ToString(LogLevel::Level level)
{
    switch (level)
    {
#define XX(name)         \
case LogLevel::name: \
    return #name;    \
    break;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}
Logger::Logger(const std::string &name)
    : m_name(name)
{
}

class MessageFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << logger->getName();
    }
};

class ThreadFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getThreadId();
    }
};

class FiberFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
        : m_format(format)
    {
    }
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getTime();
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem
{
public:
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os <<std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string &str)
        : m_string(str) {}
    void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << m_string;
    }
private:
    std::string m_string;
};

void Logger::addAppender(LogAppender::ptr appender)
{
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    for (auto it = m_appenders.begin();
            it != m_appenders.end(); ++it)
    {
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, const LogEvent::ptr event)
{
    if (level >= m_level)
    {
        for (auto &i : m_appenders)
        {
            i->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event)
{
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event)
{
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event)
{
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event)
{
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event)
{
    log(LogLevel::FATAL, event);
}

FileLogAppender::FileLogAppender(const std::string &filename)
    : m_filename(filename)
{
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event)
{
    if (level >= m_level)
    {
        std::cout << m_formatter->format(logger, m_level, event);
    }
}

bool FileLogAppender::reopen()
{
    if (m_filestream)
    {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return m_filestream.is_open();
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event)
{
    if (level >= m_level)
    {
        std::cout << m_formatter->format(logger, m_level, event);
    }
}

LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern)
{
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto &i : m_items)
    {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

//%xxx %xxx{xxx} %%
void LogFormatter::init()
{
    // str format type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); i++)
    {
        if (m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size())
        {
            if (m_pattern[i + 1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string fmt;
        std::string str;
        while (n < m_pattern.size())
        {
            if (isalpha(m_pattern[n]))
            {
                break;
            }
            if (fmt_status == 0)
            {

                if (m_pattern[n] == '{')
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1; // 解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            if (fmt_status == 1)
            {
                if (m_pattern[n] == '}')
                {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 2; // 解析格式
                    break;
                }
            }
        }
        if (fmt_status == 0)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, fmt, 0));
            }
            str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }
        else if (fmt_status == 1)
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 1));
        }
        else if (fmt_status == 2)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, fmt, 0));
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }
    }

    if (!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

        //%m -- 消息体
        //%p -- level
        //%r -- 启动后time
        //%c -- 日志名字
        //%t -- 线程id
        //%n -- 回车换行
        //%d -- 时间
        //%f -- 文件名
        //%l -- 行号
static std::map<std::string, std::function<FormatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str,C) \
    {#str,[](const std::string &fmt){return FormatItem::ptr(new C(fmt));}}
    XX(m, MessageFormatItem),
    XX(p, LevelFormatItem),
    XX(r, ElapseFormatItem),
    XX(c, NameFormatItem),
    XX(t, ThreadFormatItem),
    XX(n, NewLineFormatItem),
    XX(d, DateTimeFormatItem),
    XX(f, FilenameFormatItem),
    XX(l, LineFormatItem),
#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            // 如果第三个元素为0，表示这是一个普通字符串，不是格式化项
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            // 否则，这是一个格式化项
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                // 如果在s_format_items中找不到对应的格式化项，添加一个错误格式化项
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                
            } else {
                // 否则，使用找到的格式化项创建函数来创建FormatItem对象
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        std::cout << "str=" << std::get<0>(i) << ", fmt=" << std::get<1>(i) << ", type=" << std::get<2>(i) << std::endl;
    }
}

    
}