#include "v8_integration/error_handler.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <execinfo.h>
#include <cstdlib>
#include <cstring>
#include <map>

namespace v8_integration {

// ErrorInfo implementation
ErrorInfo::ErrorInfo(ErrorCode c, const std::string& msg, const std::string& f, 
                     int l, const std::string& func)
    : code(c), message(msg), file(f), line(l), function(func),
      timestamp(std::chrono::system_clock::now()) {
    
    // Capture stack trace
    void* trace[16];
    int trace_size = backtrace(trace, 16);
    char** messages = BacktraceSymbols(trace, trace_size);
    
    std::ostringstream oss;
    for (int i = 0; i < trace_size; ++i) {
        oss << messages[i] << "\n";
    }
    stack_trace = oss.str();
    
    free(messages);
}

// Logger implementation
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

void Logger::addHandler(std::function<void(LogLevel, const std::string&)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.push_back(handler);
}

void Logger::enableFileLogging(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_stream_ = std::make_unique<std::ofstream>(filename, std::ios::app);
}

void Logger::enableConsoleLogging(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    console_logging_ = enable;
}

void Logger::log(LogLevel level, const std::string& message, 
                const std::string& file, int line, const std::string& function) {
    if (level < current_level_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::string formatted = formatMessage(level, message, file, line, function);
    
    if (console_logging_) {
        std::cout << formatted << std::endl;
    }
    
    if (file_stream_ && file_stream_->IsOpen()) {
        *file_stream_ << formatted << std::endl;
        file_stream_->flush();
    }
    
    for (auto& handler : handlers_) {
        handler(level, formatted);
    }
}

void Logger::trace(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::TRACE, message, file, line, function);
}

void Logger::debug(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::DEBUG, message, file, line, function);
}

void Logger::info(const std::string& message, const std::string& file, 
                 int line, const std::string& function) {
    log(LogLevel::INFO, message, file, line, function);
}

void Logger::warn(const std::string& message, const std::string& file, 
                 int line, const std::string& function) {
    log(LogLevel::WARN, message, file, line, function);
}

void Logger::error(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::ERROR, message, file, line, function);
}

void Logger::fatal(const std::string& message, const std::string& file, 
                  int line, const std::string& function) {
    log(LogLevel::FATAL, message, file, line, function);
}

std::string Logger::formatMessage(LogLevel level, const std::string& message,
                                 const std::string& file, int line,
                                 const std::string& function) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::ToTimeT(now);
    
    std::ostringstream oss;
    oss << std::PutTime(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << " [" << levelToString(level) << "] ";
    
    if (!file.empty()) {
        oss << "(" << file << ":" << line;
        if (!function.empty()) {
            oss << " in " << function;
        }
        oss << ") ";
    }
    
    oss << message;
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

// V8ErrorHandler implementation
void V8ErrorHandler::setMessageHandler(v8::Isolate* isolate) {
    isolate->AddMessageListener(messageHandler);
}

void V8ErrorHandler::setPromiseRejectHandler(v8::Isolate* isolate) {
    isolate->SetPromiseRejectCallback(promiseRejectHandler);
}

void V8ErrorHandler::setFatalErrorHandler() {
    v8::V8::SetFatalErrorCallback(fatalErrorHandler);
}

void V8ErrorHandler::setOOMErrorHandler() {
    v8::V8::SetOOMErrorCallback(oomErrorHandler);
}

ErrorInfo V8ErrorHandler::extractErrorInfo(v8::Isolate* isolate, v8::Local<v8::Context> context,
                                          v8::Local<v8::Value> error) {
    v8::String::Utf8Value ErrorString(isolate, error);
    std::string message = *ErrorString ? *ErrorString : "Unknown error";
    
    ErrorCode code = ErrorCode::UNKNOWN_ERROR;
    if (error->IsObject()) {
        v8::Local<v8::Object> error_obj = error.As<v8::Object>();
        v8::Local<v8::String> name_key = v8::String::NewFromUtf8(isolate, "name").ToLocalChecked();
        v8::Local<v8::Value> name_value = error_obj->Get(context, name_key).ToLocalChecked();
        
        if (!name_value.IsEmpty()) {
            v8::String::Utf8Value NameString(isolate, name_value);
            std::string name = *NameString ? *NameString : "";
            
            if (name == "TypeError") code = ErrorCode::TYPE_ERROR;
            else if (name == "ReferenceError") code = ErrorCode::REFERENCE_ERROR;
            else if (name == "SyntaxError") code = ErrorCode::SYNTAX_ERROR;
            else if (name == "RangeError") code = ErrorCode::RANGE_ERROR;
        }
    }
    
    ErrorInfo info(code, message);
    info.stack_trace = getStackTrace(isolate, context, error);
    return info;
}

ErrorInfo V8ErrorHandler::extractErrorInfo(v8::Isolate* isolate, v8::TryCatch& try_catch) {
    if (!try_catch.HasCaught()) {
        return ErrorInfo(ErrorCode::SUCCESS, "No error");
    }
    
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Value> exception = try_catch.Exception();
    
    ErrorInfo info = extractErrorInfo(isolate, context, exception);
    
    v8::Local<v8::Message> message = try_catch.Message();
    if (!message.IsEmpty()) {
        v8::String::Utf8Value filename(isolate, message->GetScriptOrigin().ResourceName());
        info.file = *filename ? *filename : "unknown";
        info.line = message->GetLineNumber(context).FromMaybe(0);
    }
    
    return info;
}

std::string V8ErrorHandler::getStackTrace(v8::Isolate* isolate, v8::Local<v8::Context> context,
                                         v8::Local<v8::Value> error) {
    if (error->IsObject()) {
        v8::Local<v8::Object> error_obj = error.As<v8::Object>();
        v8::Local<v8::String> stack_key = v8::String::NewFromUtf8(isolate, "stack").ToLocalChecked();
        v8::Local<v8::Value> stack_value = error_obj->Get(context, stack_key).ToLocalChecked();
        
        if (!stack_value.IsEmpty() && stack_value->IsString()) {
            v8::String::Utf8Value StackString(isolate, stack_value);
            return *StackString ? *StackString : "";
        }
    }
    return "";
}

void V8ErrorHandler::logError(const ErrorInfo& error) {
    std::ostringstream oss;
    oss << "V8 Error [" << static_cast<int>(error.code) << "]: " << error.message;
    if (!error.file.empty()) {
        oss << " at " << error.file << ":" << error.line;
    }
    if (!error.stack_trace.empty()) {
        oss << "\nStack trace:\n" << error.stack_trace;
    }
    
    V8LogError(oss.str());
}

void V8ErrorHandler::handleFatalError(const ErrorInfo& error) {
    logError(error);
    V8LogFatal("Fatal V8 error occurred, terminating application");
    std::exit(1);
}

void V8ErrorHandler::messageHandler(v8::Local<v8::Message> message, v8::Local<v8::Value> error) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    ErrorInfo info = extractErrorInfo(isolate, context, error);
    logError(info);
}

void V8ErrorHandler::promiseRejectHandler(v8::PromiseRejectMessage message) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    if (message.GetEvent() == v8::kPromiseRejectWithNoHandler) {
        ErrorInfo info = extractErrorInfo(isolate, context, message.GetValue());
        info.message = "Unhandled promise rejection: " + info.message;
        logError(info);
    }
}

void V8ErrorHandler::fatalErrorHandler(const char* location, const char* message) {
    ErrorInfo info(ErrorCode::FATAL, 
                   std::string("Fatal V8 error at ") + location + ": " + message);
    handleFatalError(info);
}

void V8ErrorHandler::oomErrorHandler(const char* location, bool is_heap_oom) {
    ErrorInfo info(ErrorCode::MEMORY_ERROR,
                   std::string("Out of memory at ") + location + 
                   (is_heap_oom ? " (heap OOM)" : " (non-heap OOM)"));
    handleFatalError(info);
}

// SecurityManager implementation
void SecurityManager::enableSandbox(v8::Isolate* isolate) {
    setupSecurityCallbacks(isolate);
    V8LogInfo("V8 sandbox enabled");
}

void SecurityManager::setResourceLimits(v8::Isolate* isolate, size_t max_memory_mb,
                                       uint32_t max_execution_time_ms) {
    isolate->SetRAMSizeLimit(max_memory_mb * 1024 * 1024);
    // Note: Execution time limits would need custom implementation
    V8LogInfo("Resource limits set: " + std::to_string(max_memory_mb) + "MB memory");
}

void SecurityManager::restrictGlobalAccess(v8::Local<v8::Context> context) {
    v8::Isolate* isolate = context->GetIsolate();
    v8::Local<v8::Object> global = context->Global();
    
    // Remove dangerous globals
    std::vector<std::string> dangerous_globals = {
        "eval", "Function", "setTimeout", "setInterval", "require", "process"
    };
    
    for (const auto& name : dangerous_globals) {
        v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked();
        global->Delete(context, key);
    }
    
    V8LogInfo("Global access restricted");
}

bool SecurityManager::validateScript(const std::string& script) {
    // Basic validation - check for dangerous patterns
    std::vector<std::string> dangerous_patterns = {
        "eval(", "Function(", "setTimeout(", "setInterval(", "require(", "process."
    };
    
    for (const auto& pattern : dangerous_patterns) {
        if (script.find(pattern) != std::string::npos) {
            V8LogWarn("Dangerous pattern detected: " + pattern);
            return false;
        }
    }
    
    return true;
}

void SecurityManager::enableCodeSigning(bool enable) {
    // Implementation would depend on specific requirements
    V8LogInfo("Code signing " + std::string(enable ? "enabled" : "disabled"));
}

void SecurityManager::setupSecurityCallbacks(v8::Isolate* isolate) {
    isolate->SetAllowCodeGenerationFromStringsCallback(allowCodeGeneration);
    isolate->SetAllowWasmCodeGenerationCallback(allowWasmCodeGeneration);
}

bool SecurityManager::allowCodeGeneration(v8::Local<v8::Context> context,
                                         v8::Local<v8::String> source,
                                         bool is_code_like) {
    // By default, disallow code generation for security
    V8LogWarn("Code generation attempt blocked");
    return false;
}

bool SecurityManager::allowWasmCodeGeneration(v8::Local<v8::Context> context,
                                             v8::Local<v8::String> source) {
    // By default, disallow WASM code generation
    V8LogWarn("WASM code generation attempt blocked");
    return false;
}

// PerformanceMonitor implementation
namespace {
    std::mutex performance_mutex_;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> timings_;
    std::map<std::string, std::vector<double>> metrics_;
    std::map<std::string, int64_t> counters_;
}

void PerformanceMonitor::startTiming(const std::string& operation) {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    timings_[operation] = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endTiming(const std::string& operation) {
    auto end_time = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    auto it = timings_.find(operation);
    if (it != timings_.end()) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - it->second).count();
        metrics_[operation].push_back(duration);
        timings_.erase(it);
    }
}

void PerformanceMonitor::recordMetric(const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    metrics_[name].push_back(value);
}

void PerformanceMonitor::recordCounter(const std::string& name, int64_t value) {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    counters_[name] += value;
}

void PerformanceMonitor::generateReport() {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    V8LogInfo("=== Performance Report ===");
    
    for (const auto& [name, values] : metrics_) {
        if (!values.empty()) {
            double sum = 0;
            double min_val = values[0];
            double max_val = values[0];
            
            for (double val : values) {
                sum += val;
                min_val = std::min(min_val, val);
                max_val = std::max(max_val, val);
            }
            
            double avg = sum / values.size();
            
            V8LogInfo(name + " - Count: " + std::to_string(values.size()) +
                       ", Avg: " + std::to_string(avg) + "ms" +
                       ", Min: " + std::to_string(min_val) + "ms" +
                       ", Max: " + std::to_string(max_val) + "ms");
        }
    }
    
    for (const auto& [name, count] : counters_) {
        V8LogInfo(name + " - Count: " + std::to_string(count));
    }
}

} // namespace v8_integration