/*
 * Copyright 2016 Mingyu Gao
 *
 */
#ifndef UTILS_LOG_H_
#define UTILS_LOG_H_

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>

#define PANIC_EXIT_CODE (112)

/** @cond INTERNAL */

// Generic logging, thread-unsafe
#define __panic(logFdErr, logHeader, ...) \
{ \
    fprintf(logFdErr, "%sPanic on %s:%d: ", logHeader, __FILE__, __LINE__); \
    fprintf(logFdErr, __VA_ARGS__); \
    fprintf(logFdErr, "\n"); \
    fflush(logFdErr); \
    exit(PANIC_EXIT_CODE); \
}

#define __warn(logFdErr, logHeader, ...) \
{ \
    fprintf(logFdErr, "%sWARN: ", logHeader); \
    fprintf(logFdErr, __VA_ARGS__); \
    fprintf(logFdErr, "\n"); \
    fflush(logFdErr); \
}

#define __info(logFdOut, logHeader, ...) \
{ \
    fprintf(logFdOut, "%s", logHeader); \
    fprintf(logFdOut, __VA_ARGS__); \
    fprintf(logFdOut, "\n"); \
    fflush(logFdOut); \
}

/** @endcond */

/**
 * @addtogroup log Logging
 *
 * @{
 */

/**
 * @name
 * Basic logging.
 *
 * Thread-unsafe, print to stdout/stderr, no header.
 */
/**@{*/
#define panic(...) __panic(stderr, "", __VA_ARGS__)
#define warn(...)  __warn(stderr, "", __VA_ARGS__)
#define info(...)  __info(stdout, "", __VA_ARGS__)
/**@}*/

/**
 * @brief
 * Logging class.
 *
 * Thread-safe, support redirection to files, support header.
 */
class Logger {
public:
    /**
     * Initialize Logger.
     *
     * @param header    header for the log.
     * @param file      log file. Use stdout/stderr if nullptr.
     */
    explicit Logger(const char* header = "", const char* file = nullptr)
            : logHeader_(header) {
        if (file) {
            fd_ = fopen(file, "a");
            if (fd_ == NULL) {
                perror("fopen() failed");
                // We can panic in InitLog (will dump to stderr)
                panic("Could not open log file %s", file);
            }
            logFdOut_ = fd_;
            logFdErr_ = fd_;
        } else {
            fd_ = nullptr;
            logFdOut_ = stdout;
            logFdErr_ = stderr;
        }
    }

    ~Logger() {
        fclose(fd_);
    }

    /**
     * @name
     * Logging.
     */
    /**@{*/

    template<typename... Args>
    void log_panic(const char* fmt, Args... args) {
        __panic(logFdErr_, logHeader_.c_str(), fmt, args...);
    }

    template<typename... Args>
    void log_warn(const char* fmt, Args... args) {
        logPrintLock_.lock();
        __warn(logFdErr_, logHeader_.c_str(), fmt, args...);
        logPrintLock_.unlock();
    }

    template<typename... Args>
    void log_info(const char* fmt, Args... args) {
        logPrintLock_.lock();
        __info(logFdErr_, logHeader_.c_str(), fmt, args...);
        logPrintLock_.unlock();
    }

    /**@}*/

private:
    FILE* fd_;
    FILE* logFdErr_;
    FILE* logFdOut_;
    const std::string logHeader_;
    std::mutex logPrintLock_;
};

/**
 * @name
 * Assertion.
 */
/**@{*/

#ifndef NASSERT

#ifndef assert
#define assert(expr) \
if (!(expr)) { \
    fprintf(stderr, "%sFailed assertion on %s:%d '%s'\n", "", \
            __FILE__, __LINE__, #expr); \
    fflush(stderr); \
    exit(PANIC_EXIT_CODE); \
}
#endif

#define assert_msg(cond, ...) \
if (!(cond)) { \
    fprintf(stderr, "%sFailed assertion on %s:%d: ", "", \
            __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    fflush(stderr); \
    exit(PANIC_EXIT_CODE); \
}

#else  // NASSERT

#ifndef assert
// Avoid unused warnings, never emit any code
// see http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/
#define assert(cond) do { (void)sizeof(cond); } while (0);
#endif

#define assert_msg(cond, ...) do { (void)sizeof(cond); } while (0);

#endif  // NASSERT

/**@}*/

/**@}*/

#endif  // UTILS_LOG_H_

