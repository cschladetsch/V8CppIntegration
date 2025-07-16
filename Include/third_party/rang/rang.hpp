#ifndef RANG_HPP
#define RANG_HPP

#if defined(__unix__) || defined(__unix) || defined(__linux__)
#define OS_LINUX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#define OS_WIN
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_MAC
#else
#error unsupported platform
#endif

#if defined(OS_LINUX) || defined(OS_MAC)
#include <unistd.h>
#elif defined(OS_WIN)
#include <windows.h>
#include <io.h>
#endif

#include <iostream>
#include <type_traits>
#include <cstring>

namespace rang {

/* For better compability with most of terminals do not use any style settings
 * except of reset, bold and reversed.
 * Note that on Windows terminals bold style is same as hiColor.
 * Don't use any \033[0m directly. Use rang::style::reset instead.
 */
enum class style {
    reset     = 0,
    bold      = 1,
    dim       = 2,
    italic    = 3,
    underline = 4,
    blink     = 5,
    rblink    = 6,
    reversed  = 7,
    conceal   = 8,
    crossed   = 9
};

enum class fg {
    black   = 30,
    red     = 31,
    green   = 32,
    yellow  = 33,
    blue    = 34,
    magenta = 35,
    cyan    = 36,
    gray    = 37,
    reset   = 39
};

enum class bg {
    black   = 40,
    red     = 41,
    green   = 42,
    yellow  = 43,
    blue    = 44,
    magenta = 45,
    cyan    = 46,
    gray    = 47,
    reset   = 49
};

enum class fgB {
    black   = 90,
    red     = 91,
    green   = 92,
    yellow  = 93,
    blue    = 94,
    magenta = 95,
    cyan    = 96,
    gray    = 97
};

enum class bgB {
    black   = 100,
    red     = 101,
    green   = 102,
    yellow  = 103,
    blue    = 104,
    magenta = 105,
    cyan    = 106,
    gray    = 107
};

enum class control {
    autoColor = 0,
    forceColor = 1,
    off = 2
};

namespace rang_implementation {

inline std::streambuf const *&coutBuf() {
    static std::streambuf const *coutBuf = std::cout.rdbuf();
    return coutBuf;
}

inline std::streambuf const *&clogBuf() {
    static std::streambuf const *clogBuf = std::clog.rdbuf();
    return clogBuf;
}

inline std::streambuf const *&cerrBuf() {
    static std::streambuf const *cerrBuf = std::cerr.rdbuf();
    return cerrBuf;
}

inline bool supportsColor() {
#if defined(OS_LINUX) || defined(OS_MAC)

    static bool result = [] {
        const char *Terms[] = {
            "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm",
            "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm"
        };

        const char *env_p = std::getenv("TERM");
        if (env_p == nullptr) {
            return false;
        }

        for (auto Term : Terms) {
            if (std::strstr(env_p, Term) != nullptr) {
                return true;
            }
        }

        return false;
    }();

    return result;

#elif defined(OS_WIN)
    // All windows terminals supports color through Win32 API
    return true;
#endif
}

inline bool isTerminal(const std::streambuf *osbuf) {
    using std::cerr;
    using std::clog;
    using std::cout;
#if defined(OS_LINUX) || defined(OS_MAC)
    if (osbuf == coutBuf()) {
        return isatty(fileno(stdout)) ? true : false;
    }

    if (osbuf == cerrBuf() || osbuf == clogBuf()) {
        return isatty(fileno(stderr)) ? true : false;
    }
#elif defined(OS_WIN)
    if (osbuf == coutBuf()) {
        return _isatty(_fileno(stdout)) ? true : false;
    }

    if (osbuf == cerrBuf() || osbuf == clogBuf()) {
        return _isatty(_fileno(stderr)) ? true : false;
    }
#endif
    return false;
}

template <typename T>
using enableStd = typename std::enable_if<
    std::is_same<T, rang::style>::value ||
    std::is_same<T, rang::fg>::value ||
    std::is_same<T, rang::bg>::value ||
    std::is_same<T, rang::fgB>::value ||
    std::is_same<T, rang::bgB>::value,
    std::ostream &>::type;

} // namespace rang_implementation

template <typename T>
inline rang_implementation::enableStd<T> operator<<(std::ostream &os, T const value) {
    const control option = control::autoColor;
    switch (option) {
        case control::autoColor:
            return rang_implementation::supportsColor() &&
                   rang_implementation::isTerminal(os.rdbuf()) ?
                   os << "\033[" << static_cast<int>(value) << "m" : os;
        case control::forceColor:
            return os << "\033[" << static_cast<int>(value) << "m";
        case control::off:
            return os;
    }
    return os;
}

inline void setControlMode(control const mode) {
    // Implementation left empty for simplicity
    // In a full implementation, this would set a global control mode
}

} // namespace rang

#endif // RANG_HPP