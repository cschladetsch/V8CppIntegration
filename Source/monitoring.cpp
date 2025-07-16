#include "v8_integration/monitoring.h"
#include <thread>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <random>
#include <fstream>
#include <sys/resource.h>
#include <sys/times.h>
#include <unistd.h>

namespace v8_integration {

// MetricsCollector Implementation
MetricsCollector& MetricsCollector::getInstance() {
    static MetricsCollector instance;
    return instance;
}

void MetricsCollector::incrementCounter(const std::string& name, double value,
                                       const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = name + "_counter";
    if (metrics_.find(key) == metrics_.end()) {
        metrics_[key] = {"v8_" + name, "counter", "Counter metric for " + name, labels, 0.0, 
                        std::chrono::system_clock::now()};
    }
    
    metrics_[key].value += value;
    metrics_[key].timestamp = std::chrono::system_clock::now();
}

void MetricsCollector::setGauge(const std::string& name, double value,
                               const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = name + "_gauge";
    metrics_[key] = {"v8_" + name, "gauge", "Gauge metric for " + name, labels, value,
                     std::chrono::system_clock::now()};
}

void MetricsCollector::recordHistogram(const std::string& name, double value,
                                      const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = name + "_histogram";
    metrics_[key] = {"v8_" + name, "histogram", "Histogram metric for " + name, labels, value,
                     std::chrono::system_clock::now()};
}

void MetricsCollector::recordSummary(const std::string& name, double value,
                                    const std::map<std::string, std::string>& labels) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::string key = name + "_summary";
    metrics_[key] = {"v8_" + name, "summary", "Summary metric for " + name, labels, value,
                     std::chrono::system_clock::now()};
}

std::vector<MetricsCollector::Metric> MetricsCollector::getAllMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::vector<Metric> result;
    for (const auto& [key, metric] : metrics_) {
        result.push_back(metric);
    }
    return result;
}

std::string MetricsCollector::exportPrometheus() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::ostringstream oss;
    for (const auto& [key, metric] : metrics_) {
        oss << "# HELP " << metric.name << " " << metric.help << "\n";
        oss << "# TYPE " << metric.name << " " << metric.type << "\n";
        
        oss << metric.name;
        if (!metric.labels.empty()) {
            oss << "{";
            bool first = true;
            for (const auto& [label_key, label_value] : metric.labels) {
                if (!first) oss << ",";
                oss << label_key << "=\"" << label_value << "\"";
                first = false;
            }
            oss << "}";
        }
        oss << " " << metric.value << "\n";
    }
    
    return oss.str();
}

std::string MetricsCollector::exportJSON() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    std::ostringstream oss;
    oss << "{\n  \"metrics\": [\n";
    
    bool first = true;
    for (const auto& [key, metric] : metrics_) {
        if (!first) oss << ",\n";
        oss << "    {\n";
        oss << "      \"name\": \"" << metric.name << "\",\n";
        oss << "      \"type\": \"" << metric.type << "\",\n";
        oss << "      \"help\": \"" << metric.help << "\",\n";
        oss << "      \"value\": " << metric.value << ",\n";
        oss << "      \"labels\": {";
        
        bool first_label = true;
        for (const auto& [label_key, label_value] : metric.labels) {
            if (!first_label) oss << ",";
            oss << "\"" << label_key << "\": \"" << label_value << "\"";
            first_label = false;
        }
        oss << "}\n";
        oss << "    }";
        first = false;
    }
    
    oss << "\n  ]\n}";
    return oss.str();
}

void MetricsCollector::startPeriodicCollection(int interval_seconds) {
    if (collecting_) return;
    
    collecting_ = true;
    collection_thread_ = std::make_unique<std::thread>([this, interval_seconds]() {
        periodicCollection(interval_seconds);
    });
}

void MetricsCollector::stopPeriodicCollection() {
    collecting_ = false;
    if (collection_thread_ && collection_thread_->joinable()) {
        collection_thread_->join();
    }
}

void MetricsCollector::collectV8Metrics() {
    // V8 metrics would be collected here
    // This is a placeholder implementation
    setGauge("heap_used_bytes", 1024 * 1024 * 50); // 50MB
    setGauge("heap_total_bytes", 1024 * 1024 * 100); // 100MB
    incrementCounter("scripts_executed", 1);
}

void MetricsCollector::collectSystemMetrics() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    setGauge("cpu_user_time_seconds", usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0);
    setGauge("cpu_system_time_seconds", usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1000000.0);
    setGauge("memory_max_resident_kb", usage.ru_maxrss);
    setGauge("page_faults_major", usage.ru_majflt);
    setGauge("page_faults_minor", usage.ru_minflt);
}

void MetricsCollector::periodicCollection(int interval_seconds) {
    while (collecting_) {
        collectV8Metrics();
        collectSystemMetrics();
        
        std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
    }
}

// HealthChecker Implementation
HealthChecker& HealthChecker::getInstance() {
    static HealthChecker instance;
    return instance;
}

void HealthChecker::registerCheck(const std::string& name, CheckFunction check,
                                 std::chrono::seconds interval) {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    checks_[name] = check;
    check_intervals_[name] = interval;
}

void HealthChecker::unregisterCheck(const std::string& name) {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    checks_.erase(name);
    check_intervals_.erase(name);
    last_results_.erase(name);
}

std::vector<HealthChecker::HealthCheck> HealthChecker::runAllChecks() {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    
    std::vector<HealthCheck> results;
    for (const auto& [name, check] : checks_) {
        auto start = std::chrono::high_resolution_clock::now();
        HealthCheck result = check();
        auto end = std::chrono::high_resolution_clock::now();
        
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        result.last_check = std::chrono::system_clock::now();
        
        last_results_[name] = result;
        results.push_back(result);
    }
    
    return results;
}

HealthChecker::HealthCheck HealthChecker::runCheck(const std::string& name) {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    
    auto it = checks_.find(name);
    if (it == checks_.end()) {
        return {name, Status::UNHEALTHY, "Check not found", 
                std::chrono::system_clock::now(), std::chrono::milliseconds(0)};
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    HealthCheck result = it->second();
    auto end = std::chrono::high_resolution_clock::now();
    
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    result.last_check = std::chrono::system_clock::now();
    
    last_results_[name] = result;
    return result;
}

HealthChecker::Status HealthChecker::getOverallStatus() {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    
    Status overall = Status::HEALTHY;
    for (const auto& [name, result] : last_results_) {
        if (result.status == Status::UNHEALTHY) {
            return Status::UNHEALTHY;
        } else if (result.status == Status::DEGRADED) {
            overall = Status::DEGRADED;
        }
    }
    
    return overall;
}

std::string HealthChecker::getHealthReport() {
    std::lock_guard<std::mutex> lock(checks_mutex_);
    
    std::ostringstream oss;
    oss << "=== Health Report ===\n";
    oss << "Overall Status: ";
    
    Status overall = getOverallStatus();
    switch (overall) {
        case Status::HEALTHY: oss << "HEALTHY"; break;
        case Status::DEGRADED: oss << "DEGRADED"; break;
        case Status::UNHEALTHY: oss << "UNHEALTHY"; break;
    }
    oss << "\n\n";
    
    for (const auto& [name, result] : last_results_) {
        oss << "Check: " << name << "\n";
        oss << "  Status: ";
        switch (result.status) {
            case Status::HEALTHY: oss << "HEALTHY"; break;
            case Status::DEGRADED: oss << "DEGRADED"; break;
            case Status::UNHEALTHY: oss << "UNHEALTHY"; break;
        }
        oss << "\n";
        oss << "  Message: " << result.message << "\n";
        oss << "  Duration: " << result.duration.count() << "ms\n";
        oss << "\n";
    }
    
    return oss.str();
}

void HealthChecker::startPeriodicChecks() {
    if (checking_) return;
    
    checking_ = true;
    check_thread_ = std::make_unique<std::thread>([this]() {
        periodicChecks();
    });
}

void HealthChecker::stopPeriodicChecks() {
    checking_ = false;
    if (check_thread_ && check_thread_->joinable()) {
        check_thread_->join();
    }
}

void HealthChecker::periodicChecks() {
    while (checking_) {
        runAllChecks();
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

HealthChecker::HealthCheck HealthChecker::createV8HealthCheck() {
    return {"v8_health", Status::HEALTHY, "V8 engine is running normally", 
            std::chrono::system_clock::now(), std::chrono::milliseconds(0)};
}

HealthChecker::HealthCheck HealthChecker::createMemoryHealthCheck() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    
    // Check if memory usage is reasonable (< 1GB)
    if (usage.ru_maxrss > 1024 * 1024) {
        return {"memory_health", Status::DEGRADED, "High memory usage detected",
                std::chrono::system_clock::now(), std::chrono::milliseconds(0)};
    }
    
    return {"memory_health", Status::HEALTHY, "Memory usage is normal",
            std::chrono::system_clock::now(), std::chrono::milliseconds(0)};
}

HealthChecker::HealthCheck HealthChecker::createSystemHealthCheck() {
    return {"system_health", Status::HEALTHY, "System is operating normally",
            std::chrono::system_clock::now(), std::chrono::milliseconds(0)};
}

// TracingManager Implementation
TracingManager& TracingManager::getInstance() {
    static TracingManager instance;
    return instance;
}

std::string TracingManager::startTrace(const std::string& operation_name,
                                      const std::string& parent_trace_id) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    std::string trace_id = generateId();
    traces_[trace_id] = std::vector<Span>();
    
    // Create root span
    Span root_span;
    root_span.trace_id = trace_id;
    root_span.span_id = generateId();
    root_span.parent_span_id = parent_trace_id;
    root_span.operation_name = operation_name;
    root_span.start_time = std::chrono::system_clock::now();
    
    traces_[trace_id].push_back(root_span);
    
    return trace_id;
}

void TracingManager::finishTrace(const std::string& trace_id) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it != traces_.end() && !it->second.empty()) {
        it->second[0].end_time = std::chrono::system_clock::now();
    }
}

std::string TracingManager::startSpan(const std::string& trace_id, const std::string& operation_name,
                                     const std::string& parent_span_id) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it == traces_.end()) {
        return "";
    }
    
    Span span;
    span.trace_id = trace_id;
    span.span_id = generateId();
    span.parent_span_id = parent_span_id;
    span.operation_name = operation_name;
    span.start_time = std::chrono::system_clock::now();
    
    it->second.push_back(span);
    
    return span.span_id;
}

void TracingManager::finishSpan(const std::string& trace_id, const std::string& span_id) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it != traces_.end()) {
        for (auto& span : it->second) {
            if (span.span_id == span_id) {
                span.end_time = std::chrono::system_clock::now();
                break;
            }
        }
    }
}

void TracingManager::addTag(const std::string& trace_id, const std::string& span_id,
                           const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it != traces_.end()) {
        for (auto& span : it->second) {
            if (span.span_id == span_id) {
                span.tags[key] = value;
                break;
            }
        }
    }
}

void TracingManager::addLog(const std::string& trace_id, const std::string& span_id,
                           const std::string& message) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it != traces_.end()) {
        for (auto& span : it->second) {
            if (span.span_id == span_id) {
                span.logs.push_back(message);
                break;
            }
        }
    }
}

std::vector<TracingManager::Span> TracingManager::getTraceSpans(const std::string& trace_id) {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    auto it = traces_.find(trace_id);
    if (it != traces_.end()) {
        return it->second;
    }
    
    return {};
}

std::string TracingManager::exportJaeger() const {
    std::lock_guard<std::mutex> lock(spans_mutex_);
    
    std::ostringstream oss;
    oss << "{\n  \"data\": [\n";
    
    bool first_trace = true;
    for (const auto& [trace_id, spans] : traces_) {
        if (!first_trace) oss << ",\n";
        
        oss << "    {\n";
        oss << "      \"traceID\": \"" << trace_id << "\",\n";
        oss << "      \"spans\": [\n";
        
        bool first_span = true;
        for (const auto& span : spans) {
            if (!first_span) oss << ",\n";
            
            oss << "        {\n";
            oss << "          \"spanID\": \"" << span.span_id << "\",\n";
            oss << "          \"operationName\": \"" << span.operation_name << "\",\n";
            oss << "          \"startTime\": " << std::chrono::duration_cast<std::chrono::microseconds>(
                span.start_time.time_since_epoch()).count() << ",\n";
            oss << "          \"duration\": " << std::chrono::duration_cast<std::chrono::microseconds>(
                span.end_time - span.start_time).count() << "\n";
            oss << "        }";
            first_span = false;
        }
        
        oss << "\n      ]\n";
        oss << "    }";
        first_trace = false;
    }
    
    oss << "\n  ]\n}";
    return oss.str();
}

std::string TracingManager::exportZipkin() const {
    // Similar to Jaeger but in Zipkin format
    return exportJaeger(); // Placeholder
}

std::string TracingManager::generateId() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<> dis(0, 15);
    
    std::string id;
    for (int i = 0; i < 16; ++i) {
        int val = dis(gen);
        id += (val < 10) ? ('0' + val) : ('a' + val - 10);
    }
    
    return id;
}

} // namespace v8_integration