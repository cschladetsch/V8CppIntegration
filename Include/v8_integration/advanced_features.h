#pragma once

#include <v8.h>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>

namespace v8_integration {

// WebAssembly Integration
class WebAssemblyManager {
public:
    static void initialize(v8::Isolate* isolate);
    static v8::Local<v8::Value> compileWasm(v8::Isolate* isolate, 
                                           const std::vector<uint8_t>& wasm_bytes);
    static v8::Local<v8::Value> instantiateWasm(v8::Isolate* isolate,
                                               v8::Local<v8::Value> module,
                                               v8::Local<v8::Object> imports = {});
    static bool validateWasm(const std::vector<uint8_t>& wasm_bytes);
    static std::vector<uint8_t> loadWasmFile(const std::string& filename);
    
private:
    static void wasmCompileCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void wasmInstantiateCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Async/Await and Promise Support
class AsyncManager {
public:
    static void initialize(v8::Isolate* isolate);
    static v8::Local<v8::Promise> createPromise(v8::Isolate* isolate);
    static void resolvePromise(v8::Isolate* isolate, v8::Local<v8::Promise::Resolver> resolver,
                              v8::Local<v8::Value> value);
    static void rejectPromise(v8::Isolate* isolate, v8::Local<v8::Promise::Resolver> resolver,
                             v8::Local<v8::Value> reason);
    
    // Execute async operations
    template<typename Func>
    static v8::Local<v8::Promise> executeAsync(v8::Isolate* isolate, Func&& func);
    
    static void setTimeout(v8::Isolate* isolate, v8::Local<v8::Function> callback,
                          int timeout_ms);
    static void setInterval(v8::Isolate* isolate, v8::Local<v8::Function> callback,
                           int interval_ms);
    
private:
    static void setTimeoutCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void setIntervalCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Module System (ES6 modules and CommonJS)
class ModuleManager {
public:
    static void initialize(v8::Isolate* isolate);
    static v8::MaybeLocal<v8::Module> compileModule(v8::Isolate* isolate,
                                                   const std::string& source,
                                                   const std::string& filename);
    static v8::MaybeLocal<v8::Value> evaluateModule(v8::Isolate* isolate,
                                                   v8::Local<v8::Module> module);
    static void addModuleResolver(std::function<std::string(const std::string&)> resolver);
    static void enableCommonJS(v8::Isolate* isolate);
    
private:
    static v8::MaybeLocal<v8::Module> moduleResolveCallback(v8::Local<v8::Context> context,
                                                           v8::Local<v8::String> specifier,
                                                           v8::Local<v8::FixedArray> import_assertions,
                                                           v8::Local<v8::Module> referrer);
    static void requireCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static std::vector<std::function<std::string(const std::string&)>> module_resolvers_;
    static std::map<std::string, v8::Global<v8::Module>> module_cache_;
};

// Worker Thread Support
class WorkerManager {
public:
    class Worker {
    public:
        Worker(v8::Isolate* parent_isolate, const std::string& script);
        ~Worker();
        
        void start();
        void terminate();
        void postMessage(v8::Local<v8::Value> message);
        void setMessageHandler(std::function<void(v8::Local<v8::Value>)> handler);
        
    private:
        std::unique_ptr<std::thread> thread_;
        std::atomic<bool> running_;
        std::queue<v8::Global<v8::Value>> message_queue_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cv_;
        std::string script_;
        std::function<void(v8::Local<v8::Value>)> message_handler_;
        
        void workerThread();
    };
    
    static void initialize(v8::Isolate* isolate);
    static std::shared_ptr<Worker> createWorker(v8::Isolate* isolate, const std::string& script);
    
private:
    static void workerConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Advanced HTTP Server Integration
class HttpServer {
public:
    struct Request {
        std::string method;
        std::string url;
        std::map<std::string, std::string> headers;
        std::string body;
        std::map<std::string, std::string> query_params;
    };
    
    struct Response {
        int status_code = 200;
        std::map<std::string, std::string> headers;
        std::string body;
    };
    
    using RequestHandler = std::function<void(const Request&, Response&)>;
    
    static void initialize(v8::Isolate* isolate);
    static void createServer(v8::Isolate* isolate, int port, RequestHandler handler);
    static void get(const std::string& path, RequestHandler handler);
    static void post(const std::string& path, RequestHandler handler);
    static void serveStatic(const std::string& path, const std::string& directory);
    
private:
    static void serverCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void httpGetCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void httpPostCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static std::map<std::string, RequestHandler> get_handlers_;
    static std::map<std::string, RequestHandler> post_handlers_;
    static std::string static_directory_;
};

// Database Integration
class DatabaseManager {
public:
    class Connection {
    public:
        virtual ~Connection() = default;
        virtual bool connect(const std::string& connection_string) = 0;
        virtual void disconnect() = 0;
        virtual v8::Local<v8::Value> query(v8::Isolate* isolate, const std::string& sql,
                                          const std::vector<v8::Local<v8::Value>>& params) = 0;
        virtual bool execute(const std::string& sql,
                           const std::vector<v8::Local<v8::Value>>& params) = 0;
    };
    
    static void initialize(v8::Isolate* isolate);
    static void registerDriver(const std::string& name, 
                              std::function<std::unique_ptr<Connection>()> factory);
    static std::unique_ptr<Connection> createConnection(const std::string& driver_name);
    
private:
    static void connectCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void queryCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static std::map<std::string, std::function<std::unique_ptr<Connection>()>> drivers_;
};

// File System Operations
class FileSystem {
public:
    static void initialize(v8::Isolate* isolate);
    static void readFile(const std::string& filename, 
                        std::function<void(bool, const std::string&)> callback);
    static void writeFile(const std::string& filename, const std::string& content,
                         std::function<void(bool)> callback);
    static void stat(const std::string& path,
                    std::function<void(bool, const std::map<std::string, v8::Local<v8::Value>>&)> callback);
    static void readDir(const std::string& path,
                       std::function<void(bool, const std::vector<std::string>&)> callback);
    
private:
    static void readFileCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void writeFileCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void statCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void readdirCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Cryptography Support
class CryptoManager {
public:
    static void initialize(v8::Isolate* isolate);
    static std::string hash(const std::string& algorithm, const std::string& data);
    static std::string hmac(const std::string& algorithm, const std::string& key,
                           const std::string& data);
    static std::string encrypt(const std::string& algorithm, const std::string& key,
                              const std::string& data);
    static std::string decrypt(const std::string& algorithm, const std::string& key,
                              const std::string& data);
    static std::string generateKey(const std::string& algorithm, int key_size);
    static std::string randomBytes(int size);
    
private:
    static void hashCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void hmacCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void encryptCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void decryptCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void generateKeyCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void randomBytesCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Performance Profiler
class Profiler {
public:
    static void initialize(v8::Isolate* isolate);
    static void startProfiling(const std::string& title);
    static void stopProfiling(const std::string& title);
    static void startCpuProfiling(const std::string& title);
    static void stopCpuProfiling(const std::string& title);
    static void takeHeapSnapshot(const std::string& filename);
    static void generateReport(const std::string& filename);
    
private:
    static void startProfilingCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void stopProfilingCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void heapSnapshotCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Event System
class EventEmitter {
public:
    using EventCallback = std::function<void(const std::vector<v8::Local<v8::Value>>&)>;
    
    void on(const std::string& event, EventCallback callback);
    void once(const std::string& event, EventCallback callback);
    void off(const std::string& event, EventCallback callback);
    void emit(const std::string& event, const std::vector<v8::Local<v8::Value>>& args);
    
    static void initialize(v8::Isolate* isolate);
    static v8::Local<v8::Object> createEventEmitter(v8::Isolate* isolate);
    
private:
    std::map<std::string, std::vector<EventCallback>> listeners_;
    std::map<std::string, std::vector<EventCallback>> once_listeners_;
    std::mutex listeners_mutex_;
    
    static void onCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void onceCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void offCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void emitCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
};

// Configuration Manager
class ConfigManager {
public:
    static void initialize(v8::Isolate* isolate);
    static void loadConfig(const std::string& filename);
    static v8::Local<v8::Value> get(v8::Isolate* isolate, const std::string& key);
    static void set(const std::string& key, v8::Local<v8::Value> value);
    static void save(const std::string& filename);
    static void watch(const std::string& key, std::function<void(v8::Local<v8::Value>)> callback);
    
private:
    static void getCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void setCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void watchCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
    static std::map<std::string, v8::Global<v8::Value>> config_;
    static std::map<std::string, std::vector<std::function<void(v8::Local<v8::Value>)>>> watchers_;
};

} // namespace v8_integration