#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <fstream>
#include <map>
#include <v8.h>

namespace v8_integration {

enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

enum class ErrorCode {
    SUCCESS = 0,
    INITIALIZATION_FAILED = 1000,
    COMPILATION_FAILED = 1001,
    EXECUTION_FAILED = 1002,
    TYPE_ERROR = 1003,
    REFERENCE_ERROR = 1004,
    SYNTAX_ERROR = 1005,
    RANGE_ERROR = 1006,
    MEMORY_ERROR = 2000,
    SECURITY_ERROR = 3000,
    TIMEOUT_ERROR = 4000,
    UNKNOWN_ERROR = 9999
};

struct ErrorInfo {
    ErrorCode code;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
    std::string stack_trace;
    
    ErrorInfo(ErrorCode c, const std::string& msg, const std::string& f = "", 
              int l = 0, const std::string& func = "");
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLevel(LogLevel level);
    void addHandler(std::function<void(LogLevel, const std::string&)> handler);
    void enableFileLogging(const std::string& filename);
    void enableConsoleLogging(bool enable = true);
    
    void log(LogLevel level, const std::string& message, 
             const std::string& file = "", int line = 0, 
             const std::string& function = "");
    
    void trace(const std::string& message, const std::string& file = "", 
               int line = 0, const std::string& function = "");
    void debug(const std::string& message, const std::string& file = "", 
               int line = 0, const std::string& function = "");
    void info(const std::string& message, const std::string& file = "", 
              int line = 0, const std::string& function = "");
    void warn(const std::string& message, const std::string& file = "", 
              int line = 0, const std::string& function = "");
    void error(const std::string& message, const std::string& file = "", 
               int line = 0, const std::string& function = "");
    void fatal(const std::string& message, const std::string& file = "", 
               int line = 0, const std::string& function = "");

private:
    Logger() = default;
    LogLevel current_level_ = LogLevel::INFO;
    std::vector<std::function<void(LogLevel, const std::string&)>> handlers_;
    std::mutex mutex_;
    std::unique_ptr<std::ofstream> file_stream_;
    bool console_logging_ = true;
    
    std::string formatMessage(LogLevel level, const std::string& message,
                             const std::string& file, int line,
                             const std::string& function);
    std::string levelToString(LogLevel level);
};

class V8ErrorHandler {
public:
    static void setMessageHandler(v8::Isolate* isolate);
    static void setPromiseRejectHandler(v8::Isolate* isolate);
    static void setFatalErrorHandler();
    static void setOOMErrorHandler();
    
    static ErrorInfo extractErrorInfo(v8::Isolate* isolate, v8::Local<v8::Context> context,
                                     v8::Local<v8::Value> error);
    static ErrorInfo extractErrorInfo(v8::Isolate* isolate, v8::TryCatch& try_catch);
    
    static std::string getStackTrace(v8::Isolate* isolate, v8::Local<v8::Context> context,
                                    v8::Local<v8::Value> error);
    
    static void logError(const ErrorInfo& error);
    static void handleFatalError(const ErrorInfo& error);

private:
    static void messageHandler(v8::Local<v8::Message> message, v8::Local<v8::Value> error);
    static void promiseRejectHandler(v8::PromiseRejectMessage message);
    static void fatalErrorHandler(const char* location, const char* message);
    static void oomErrorHandler(const char* location, bool is_heap_oom);
};

class SecurityManager {
public:
    static void enableSandbox(v8::Isolate* isolate);
    static void setResourceLimits(v8::Isolate* isolate, size_t max_memory_mb,
                                 uint32_t max_execution_time_ms);
    static void restrictGlobalAccess(v8::Local<v8::Context> context);
    static bool validateScript(const std::string& script);
    static void enableCodeSigning(bool enable);
    
private:
    static void setupSecurityCallbacks(v8::Isolate* isolate);
    static bool allowCodeGeneration(v8::Local<v8::Context> context,
                                   v8::Local<v8::String> source,
                                   bool is_code_like);
    static bool allowWasmCodeGeneration(v8::Local<v8::Context> context,
                                       v8::Local<v8::String> source);
};

class PerformanceMonitor {
public:
    static void startTiming(const std::string& operation);
    static void endTiming(const std::string& operation);
    static void recordMetric(const std::string& name, double value);
    static void recordCounter(const std::string& name, int64_t value = 1);
    static void generateReport();
    
private:
    static std::mutex mutex_;
    static std::map<std::string, std::chrono::high_resolution_clock::time_point> timings_;
    static std::map<std::string, std::vector<double>> metrics_;
    static std::map<std::string, int64_t> counters_;
};

} // namespace v8_integration

// Convenience macros
#define V8_LOG_TRACE(msg) v8_integration::Logger::getInstance().trace(msg, __FILE__, __LINE__, __FUNCTION__)
#define V8_LOG_DEBUG(msg) v8_integration::Logger::getInstance().debug(msg, __FILE__, __LINE__, __FUNCTION__)
#define V8_LOG_INFO(msg) v8_integration::Logger::getInstance().info(msg, __FILE__, __LINE__, __FUNCTION__)
#define V8_LOG_WARN(msg) v8_integration::Logger::getInstance().warn(msg, __FILE__, __LINE__, __FUNCTION__)
#define V8_LOG_ERROR(msg) v8_integration::Logger::getInstance().error(msg, __FILE__, __LINE__, __FUNCTION__)
#define V8_LOG_FATAL(msg) v8_integration::Logger::getInstance().fatal(msg, __FILE__, __LINE__, __FUNCTION__)

#define V8_PERF_START(op) v8_integration::PerformanceMonitor::startTiming(op)
#define V8_PERF_END(op) v8_integration::PerformanceMonitor::endTiming(op)
#define V8_PERF_RECORD(name, value) v8_integration::PerformanceMonitor::recordMetric(name, value)
#define V8_PERF_COUNT(name) v8_integration::PerformanceMonitor::recordCounter(name)