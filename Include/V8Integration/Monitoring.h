#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <v8.h>

namespace v8_integration {

// Metrics collection and monitoring
class MetricsCollector {
public:
    struct Metric {
        std::string name;
        std::string type; // counter, gauge, histogram, summary
        std::string help;
        std::map<std::string, std::string> labels;
        double value;
        std::chrono::system_clock::time_point timestamp;
    };
    
    static MetricsCollector& getInstance();
    
    void incrementCounter(const std::string& name, double value = 1.0, 
                         const std::map<std::string, std::string>& labels = {});
    void setGauge(const std::string& name, double value,
                  const std::map<std::string, std::string>& labels = {});
    void recordHistogram(const std::string& name, double value,
                        const std::map<std::string, std::string>& labels = {});
    void recordSummary(const std::string& name, double value,
                      const std::map<std::string, std::string>& labels = {});
    
    std::vector<Metric> getAllMetrics() const;
    std::string exportPrometheus() const;
    std::string exportJSON() const;
    
    void startPeriodicCollection(int interval_seconds);
    void stopPeriodicCollection();
    
private:
    MetricsCollector() = default;
    mutable std::mutex metrics_mutex_;
    std::map<std::string, Metric> metrics_;
    std::atomic<bool> collecting_{false};
    std::unique_ptr<std::thread> collection_thread_;
    
    void collectV8Metrics();
    void collectSystemMetrics();
    void periodicCollection(int interval_seconds);
};

// Health check system
class HealthChecker {
public:
    enum class Status {
        HEALTHY,
        DEGRADED,
        UNHEALTHY
    };
    
    struct HealthCheck {
        std::string name;
        Status status;
        std::string message;
        std::chrono::system_clock::time_point last_check;
        std::chrono::milliseconds duration;
    };
    
    using CheckFunction = std::function<HealthCheck()>;
    
    static HealthChecker& getInstance();
    
    void registerCheck(const std::string& name, CheckFunction check, 
                      std::chrono::seconds interval = std::chrono::seconds(30));
    void unregisterCheck(const std::string& name);
    
    std::vector<HealthCheck> runAllChecks();
    HealthCheck runCheck(const std::string& name);
    
    Status getOverallStatus();
    std::string getHealthReport();
    
    void startPeriodicChecks();
    void stopPeriodicChecks();
    
private:
    HealthChecker() = default;
    mutable std::mutex checks_mutex_;
    std::map<std::string, CheckFunction> checks_;
    std::map<std::string, HealthCheck> last_results_;
    std::map<std::string, std::chrono::seconds> check_intervals_;
    std::atomic<bool> checking_{false};
    std::unique_ptr<std::thread> check_thread_;
    
    void periodicChecks();
    HealthCheck createV8HealthCheck();
    HealthCheck createMemoryHealthCheck();
    HealthCheck createSystemHealthCheck();
};

// Distributed tracing support
class TracingManager {
public:
    struct Span {
        std::string trace_id;
        std::string span_id;
        std::string parent_span_id;
        std::string operation_name;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        std::map<std::string, std::string> tags;
        std::vector<std::string> logs;
    };
    
    static TracingManager& getInstance();
    
    std::string startTrace(const std::string& operation_name,
                          const std::string& parent_trace_id = "");
    void finishTrace(const std::string& trace_id);
    
    std::string startSpan(const std::string& trace_id, const std::string& operation_name,
                         const std::string& parent_span_id = "");
    void finishSpan(const std::string& trace_id, const std::string& span_id);
    
    void addTag(const std::string& trace_id, const std::string& span_id,
                const std::string& key, const std::string& value);
    void addLog(const std::string& trace_id, const std::string& span_id,
                const std::string& message);
    
    std::vector<Span> getTraceSpans(const std::string& trace_id);
    std::string exportJaeger() const;
    std::string exportZipkin() const;
    
private:
    TracingManager() = default;
    mutable std::mutex spans_mutex_;
    std::map<std::string, std::vector<Span>> traces_;
    
    std::string generateId();
};

// Performance profiler
class PerformanceProfiler {
public:
    struct ProfileData {
        std::string function_name;
        std::chrono::nanoseconds total_time;
        std::chrono::nanoseconds avg_time;
        std::chrono::nanoseconds min_time;
        std::chrono::nanoseconds max_time;
        uint64_t call_count;
        std::vector<std::chrono::nanoseconds> samples;
    };
    
    static PerformanceProfiler& getInstance();
    
    void startProfiling(const std::string& name);
    void stopProfiling(const std::string& name);
    
    void recordExecution(const std::string& function_name,
                        std::chrono::nanoseconds duration);
    
    ProfileData getProfileData(const std::string& function_name);
    std::vector<ProfileData> getAllProfileData();
    
    void enableCpuProfiling(v8::Isolate* isolate);
    void disableCpuProfiling(v8::Isolate* isolate);
    
    void enableHeapProfiling(v8::Isolate* isolate);
    void disableHeapProfiling(v8::Isolate* isolate);
    
    void takeHeapSnapshot(v8::Isolate* isolate, const std::string& filename);
    
    std::string generateReport();
    
private:
    PerformanceProfiler() = default;
    mutable std::mutex profile_mutex_;
    std::map<std::string, ProfileData> profiles_;
    std::map<std::string, std::chrono::high_resolution_clock::time_point> active_timers_;
    
    // v8::CpuProfiler* cpu_profiler_ = nullptr; // Not available in all V8 versions
    v8::HeapProfiler* heap_profiler_ = nullptr;
};

// Alert system
class AlertManager {
public:
    enum class AlertLevel {
        INFO,
        WARNING,
        CRITICAL
    };
    
    struct Alert {
        std::string id;
        AlertLevel level;
        std::string title;
        std::string description;
        std::chrono::system_clock::time_point timestamp;
        std::map<std::string, std::string> labels;
        bool resolved = false;
    };
    
    using AlertHandler = std::function<void(const Alert&)>;
    
    static AlertManager& getInstance();
    
    void registerHandler(AlertHandler handler);
    void fireAlert(AlertLevel level, const std::string& title,
                   const std::string& description,
                   const std::map<std::string, std::string>& labels = {});
    void resolveAlert(const std::string& alert_id);
    
    std::vector<Alert> getActiveAlerts();
    std::vector<Alert> getAllAlerts();
    
    void setupThresholds(const std::string& metric_name, double warning_threshold,
                        double critical_threshold);
    
private:
    AlertManager() = default;
    mutable std::mutex alerts_mutex_;
    std::vector<Alert> alerts_;
    std::vector<AlertHandler> handlers_;
    std::map<std::string, std::pair<double, double>> thresholds_;
    
    std::string generateAlertId();
    void checkThresholds();
};

// Resource monitor
class ResourceMonitor {
public:
    struct ResourceUsage {
        double cpu_usage_percent;
        uint64_t memory_usage_bytes;
        uint64_t memory_limit_bytes;
        uint64_t heap_usage_bytes;
        uint64_t heap_limit_bytes;
        uint64_t external_memory_bytes;
        uint32_t handles_count;
        uint32_t gc_count;
        std::chrono::milliseconds gc_time;
    };
    
    static ResourceMonitor& getInstance();
    
    ResourceUsage getCurrentUsage(v8::Isolate* isolate);
    void startMonitoring(v8::Isolate* isolate, std::chrono::seconds interval);
    void stopMonitoring();
    
    void setResourceLimits(uint64_t max_memory_bytes, double max_cpu_percent);
    bool checkResourceLimits(const ResourceUsage& usage);
    
    std::vector<ResourceUsage> getUsageHistory();
    
private:
    ResourceMonitor() = default;
    std::atomic<bool> monitoring_{false};
    std::unique_ptr<std::thread> monitor_thread_;
    std::mutex usage_mutex_;
    std::vector<ResourceUsage> usage_history_;
    
    uint64_t max_memory_bytes_ = 0;
    double max_cpu_percent_ = 0.0;
    
    void monitoringLoop(v8::Isolate* isolate, std::chrono::seconds interval);
    double getCpuUsage();
    uint64_t getMemoryUsage();
};

// Log aggregation and analysis
class LogAggregator {
public:
    struct LogEntry {
        std::string level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::string source;
        std::map<std::string, std::string> fields;
    };
    
    static LogAggregator& getInstance();
    
    void addEntry(const LogEntry& entry);
    std::vector<LogEntry> getEntries(const std::string& level = "",
                                    std::chrono::system_clock::time_point since = {});
    
    void enableElasticsearchExport(const std::string& endpoint);
    void enableSyslogExport(const std::string& address);
    
    void analyzePatterns();
    std::map<std::string, int> getErrorFrequency();
    
private:
    LogAggregator() = default;
    mutable std::mutex entries_mutex_;
    std::vector<LogEntry> entries_;
    
    std::string elasticsearch_endpoint_;
    std::string syslog_address_;
    
    void exportToElasticsearch(const LogEntry& entry);
    void exportToSyslog(const LogEntry& entry);
};

// Service discovery
class ServiceDiscovery {
public:
    struct ServiceInfo {
        std::string name;
        std::string address;
        int port;
        std::map<std::string, std::string> metadata;
        std::chrono::system_clock::time_point last_heartbeat;
        bool healthy;
    };
    
    static ServiceDiscovery& getInstance();
    
    void registerService(const std::string& name, const std::string& address,
                        int port, const std::map<std::string, std::string>& metadata = {});
    void deregisterService(const std::string& name);
    
    std::vector<ServiceInfo> discoverServices(const std::string& name = "");
    ServiceInfo getService(const std::string& name);
    
    void startHeartbeat(const std::string& service_name, std::chrono::seconds interval);
    void stopHeartbeat(const std::string& service_name);
    
private:
    ServiceDiscovery() = default;
    mutable std::mutex services_mutex_;
    std::map<std::string, ServiceInfo> services_;
    std::map<std::string, std::unique_ptr<std::thread>> heartbeat_threads_;
    std::atomic<bool> running_{true};
    
    void heartbeatLoop(const std::string& service_name, std::chrono::seconds interval);
};

} // namespace v8_integration