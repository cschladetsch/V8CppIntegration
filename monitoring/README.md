# Monitoring Directory

This directory contains configuration files for monitoring and observability tools.

## Files

### prometheus.yml
Prometheus configuration template for scraping metrics from V8 integration services.

**Pre-configured Jobs:**
- `v8-integration`: Main application metrics (port 8080)
- `v8-performance`: Performance metrics endpoint (port 8081)
- `v8-health`: Health check endpoint (port 8082)

## Current Status

This configuration is provided as a template for setting up monitoring. The actual metrics endpoints would need to be implemented in your application using the monitoring headers and implementations from the `include/` and `src/` directories.

**Scrape Configuration:**
```yaml
scrape_configs:
  - job_name: 'v8-integration'
    static_configs:
      - targets: ['localhost:8080']
    scrape_interval: 5s
    metrics_path: /metrics
```

## Setting Up Monitoring

### 1. Using Docker Compose
The easiest way to set up monitoring is using the provided docker-compose.yml:

```bash
docker-compose up prometheus grafana
```

This will start:
- Prometheus on http://localhost:9090
- Grafana on http://localhost:3000 (admin/admin)

### 2. Manual Setup

#### Prometheus
```bash
# Download and install Prometheus
wget https://github.com/prometheus/prometheus/releases/download/v2.37.0/prometheus-2.37.0.linux-amd64.tar.gz
tar xvf prometheus-2.37.0.linux-amd64.tar.gz

# Run with our configuration
./prometheus --config.file=monitoring/prometheus.yml
```

#### Grafana
```bash
# Install Grafana
sudo apt-get install -y software-properties-common
sudo add-apt-repository "deb https://packages.grafana.com/oss/deb stable main"
sudo apt-get update
sudo apt-get install grafana

# Start Grafana
sudo systemctl start grafana-server
```

## Example Metrics

If you implement the monitoring features using the provided headers, you could expose metrics like:

### System Metrics
- `v8_heap_used_bytes`: Current heap memory usage
- `v8_heap_total_bytes`: Total heap size
- `v8_cpu_user_time_seconds`: CPU time in user mode
- `v8_cpu_system_time_seconds`: CPU time in system mode
- `v8_memory_max_resident_kb`: Maximum resident set size

### Application Metrics
- `v8_scripts_executed_total`: Number of scripts executed
- `v8_errors_total{level="error|warning|critical"}`: Error counts by level
- `v8_request_duration_seconds`: HTTP request duration histogram
- `v8_active_contexts`: Number of active V8 contexts

### Performance Metrics
- `v8_gc_duration_seconds`: Garbage collection duration
- `v8_compilation_duration_seconds`: Script compilation time
- `v8_execution_duration_seconds`: Script execution time

## Creating Dashboards

### Grafana Dashboard
1. Add Prometheus as a data source in Grafana
2. Import the provided dashboard or create custom ones
3. Use PromQL queries to visualize metrics

Example queries:
```promql
# Request rate
rate(v8_scripts_executed_total[5m])

# Memory usage percentage
(v8_heap_used_bytes / v8_heap_total_bytes) * 100

# Error rate by level
rate(v8_errors_total[5m])
```

## Alerting Rules

Create alert rules in `monitoring/rules/alerts.yml`:

```yaml
groups:
  - name: v8_alerts
    rules:
      - alert: HighMemoryUsage
        expr: (v8_heap_used_bytes / v8_heap_total_bytes) > 0.9
        for: 5m
        annotations:
          summary: "High memory usage detected"
          
      - alert: HighErrorRate
        expr: rate(v8_errors_total{level="error"}[5m]) > 10
        for: 2m
        annotations:
          summary: "High error rate detected"
```

## Integration with Application

To integrate monitoring into your application:

1. Use the `MetricsCollector` class from `include/v8_integration/monitoring.h`
2. Implement health check endpoints using `HealthChecker`
3. Add distributed tracing with `TracingManager`
4. Aggregate logs using `LogAggregator`

See the header files and reference implementations for detailed examples.