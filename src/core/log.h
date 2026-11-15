#ifndef __LOG_H__
#define __LOG_H__

#include <cerrno>
#include <iostream>
#include <mutex>
#include <sstream>
#include <source_location>
#include <utility>

struct Log {
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    template <typename... Args>
    static void debug(Args&&... args) {
#ifndef NDEBUG
        write(Level::DEBUG, std::source_location::current(), std::forward<Args>(args)...);
#else
        (void)sizeof...(args); // silence unused-parameter warnings
#endif
    }

    template <typename... Args>
    static void info(Args&&... args) {
        write(Level::INFO, std::source_location::current(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warning(Args&&... args) {
        write(Level::WARNING, std::source_location::current(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(Args&&... args) {
        write(Level::ERROR, std::source_location::current(), std::forward<Args>(args)...);
    }

    // compatibility helpers for older call sites
    template <typename... Args>
    [[deprecated("Use Log::info() instead")]] 
    static void add(Args&&... args) {
        info(std::forward<Args>(args)...);
    }

private:
    template <typename... Args>
    static void write(Level level, std::source_location loc, Args&&... args) {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));

        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);

        std::cerr << "[" << level_name(level) << "] ";
        if (level == Level::WARNING || level == Level::ERROR) {
            std::cerr << loc.file_name() << ":" << loc.line() << ": ";
        }
        std::cerr << oss.str() << '\n';
    }

    static const char* level_name(Level level) {
        switch (level) {
            case Level::DEBUG:   return "DEBUG";
            case Level::INFO:    return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR:   return "ERROR";
        }
        return "UNKNOWN";
    }
};

#endif //__LOG_H__

