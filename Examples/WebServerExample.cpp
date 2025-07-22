#include "V8Compat.h"
#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>
#include <regex>
#include <libplatform/libplatform.h>
#include <v8.h>
#include "V8Compat.h"

// Simple HTTP server implementation using V8 for request handling
class V8WebServer {
private:
    std::unique_ptr<v8::Platform> platform_;
    v8::Isolate* isolate_;
    v8::Global<v8::Context> context_;
    std::atomic<bool> running_{false};
    std::thread server_thread_;
    std::mutex request_mutex_;
    
    struct HttpRequest {
        std::string method;
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::map<std::string, std::string> query_params;
    };
    
    struct HttpResponse {
        int status_code = 200;
        std::map<std::string, std::string> headers;
        std::string body;
    };
    
public:
    V8WebServer() {
        // Initialize V8
        v8::V8::InitializeICUDefaultLocation(".");
        v8::V8::InitializeExternalStartupData(".");
        platform_ = v8_compat::CreateDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
        
        // Create isolate and context
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate_ = v8::Isolate::New(create_params);
        
        v8::Isolate::Scope IsolateScope(isolate_);
        v8::HandleScope HandleScope(isolate_);
        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
        
        setupJavaScriptEnvironment();
    }
    
    ~V8WebServer() {
        stop();
        context_.Reset();
        isolate_->Dispose();
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
    }
    
    void setupJavaScriptEnvironment() {
        v8::Isolate::Scope IsolateScope(isolate_);
        v8::HandleScope HandleScope(isolate_);
        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate_, context_);
        v8::Context::Scope ContextScope(context);
        
        // Add console.log function
        auto console_log = [](const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();
            v8::HandleScope HandleScope(isolate);
            
            for (int i = 0; i < args.Length(); i++) {
                if (i > 0) std::cout << " ";
                v8::String::Utf8Value str(isolate, args[i]);
                std::cout << *str;
            }
            std::cout << std::endl;
        };
        
        v8::Local<v8::Object> console = v8::Object::New(isolate_);
        v8::Local<v8::Function> log_func = v8::Function::New(context, console_log).ToLocalChecked();
        console->Set(context, v8::String::NewFromUtf8(isolate_, "log").ToLocalChecked(), log_func).FromJust();
        context->Global()->Set(context, v8::String::NewFromUtf8(isolate_, "console").ToLocalChecked(), console).FromJust();
        
        // Add JSON support
        const char* json_code = R"(
            if (typeof JSON === 'undefined') {
                JSON = {
                    stringify: function(obj) {
                        return JSON.stringify(obj);
                    },
                    parse: function(str) {
                        return JSON.parse(str);
                    }
                };
            }
        )";
        
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate_, json_code).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        script->Run(context).ToLocalChecked();
    }
    
    void loadScript(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open script file: " << filename << std::endl;
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string script_content = buffer.str();
        
        v8::Isolate::Scope IsolateScope(isolate_);
        v8::HandleScope HandleScope(isolate_);
        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate_, context_);
        v8::Context::Scope ContextScope(context);
        
        v8::TryCatch TryCatch(isolate_);
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate_, script_content.c_str()).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        
        if (script.IsEmpty()) {
            v8::String::Utf8Value error(isolate_, TryCatch.Exception());
            std::cerr << "Script compilation error: " << *error << std::endl;
            return;
        }
        
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        if (result.IsEmpty()) {
            v8::String::Utf8Value error(isolate_, TryCatch.Exception());
            std::cerr << "Script execution error: " << *error << std::endl;
            return;
        }
        
        std::cout << "Script loaded successfully: " << filename << std::endl;
    }
    
    HttpResponse handleRequest(const HttpRequest& request) {
        std::lock_guard<std::mutex> lock(request_mutex_);
        
        v8::Isolate::Scope IsolateScope(isolate_);
        v8::HandleScope HandleScope(isolate_);
        v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate_, context_);
        v8::Context::Scope ContextScope(context);
        
        // Create request object
        v8::Local<v8::Object> req_obj = v8::Object::New(isolate_);
        req_obj->Set(context, v8::String::NewFromUtf8(isolate_, "method").ToLocalChecked(),
                     v8::String::NewFromUtf8(isolate_, request.method.c_str()).ToLocalChecked()).FromJust();
        req_obj->Set(context, v8::String::NewFromUtf8(isolate_, "path").ToLocalChecked(),
                     v8::String::NewFromUtf8(isolate_, request.path.c_str()).ToLocalChecked()).FromJust();
        req_obj->Set(context, v8::String::NewFromUtf8(isolate_, "body").ToLocalChecked(),
                     v8::String::NewFromUtf8(isolate_, request.body.c_str()).ToLocalChecked()).FromJust();
        
        // Create headers object
        v8::Local<v8::Object> headers_obj = v8::Object::New(isolate_);
        for (const auto& [key, value] : request.headers) {
            headers_obj->Set(context, v8::String::NewFromUtf8(isolate_, key.c_str()).ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate_, value.c_str()).ToLocalChecked()).FromJust();
        }
        req_obj->Set(context, v8::String::NewFromUtf8(isolate_, "headers").ToLocalChecked(), headers_obj).FromJust();
        
        // Create response object
        v8::Local<v8::Object> res_obj = v8::Object::New(isolate_);
        res_obj->Set(context, v8::String::NewFromUtf8(isolate_, "statusCode").ToLocalChecked(),
                     v8::Number::New(isolate_, 200)).FromJust();
        res_obj->Set(context, v8::String::NewFromUtf8(isolate_, "body").ToLocalChecked(),
                     v8::String::NewFromUtf8(isolate_, "").ToLocalChecked()).FromJust();
        
        // Add response methods
        auto set_status = [](const v8::FunctionCallbackInfo<v8::Value>& args) {
            if (args.Length() > 0) {
                v8::Local<v8::Object> self = args.This();
                self->Set(args.GetIsolate()->GetCurrentContext(),
                         v8::String::NewFromUtf8(args.GetIsolate(), "statusCode").ToLocalChecked(),
                         args[0]).FromJust();
            }
        };
        
        auto send = [](const v8::FunctionCallbackInfo<v8::Value>& args) {
            if (args.Length() > 0) {
                v8::Local<v8::Object> self = args.This();
                self->Set(args.GetIsolate()->GetCurrentContext(),
                         v8::String::NewFromUtf8(args.GetIsolate(), "body").ToLocalChecked(),
                         args[0]).FromJust();
            }
        };
        
        auto json = [](const v8::FunctionCallbackInfo<v8::Value>& args) {
            if (args.Length() > 0) {
                v8::Isolate* isolate = args.GetIsolate();
                v8::Local<v8::Context> context = isolate->GetCurrentContext();
                v8::Local<v8::Object> self = args.This();
                
                // Set content type
                v8::Local<v8::Object> headers = v8::Object::New(isolate);
                headers->Set(context, v8::String::NewFromUtf8(isolate, "Content-Type").ToLocalChecked(),
                           v8::String::NewFromUtf8(isolate, "application/json").ToLocalChecked()).FromJust();
                self->Set(context, v8::String::NewFromUtf8(isolate, "headers").ToLocalChecked(), headers).FromJust();
                
                // Stringify JSON
                v8::Local<v8::String> json_str = v8::JSON::Stringify(context, args[0]).ToLocalChecked();
                self->Set(context, v8::String::NewFromUtf8(isolate, "body").ToLocalChecked(), json_str).FromJust();
            }
        };
        
        res_obj->Set(context, v8::String::NewFromUtf8(isolate_, "status").ToLocalChecked(),
                     v8::Function::New(context, set_status).ToLocalChecked()).FromJust();
        res_obj->Set(context, v8::String::NewFromUtf8(isolate_, "send").ToLocalChecked(),
                     v8::Function::New(context, send).ToLocalChecked()).FromJust();
        res_obj->Set(context, v8::String::NewFromUtf8(isolate_, "json").ToLocalChecked(),
                     v8::Function::New(context, json).ToLocalChecked()).FromJust();
        
        // Look for request handler
        v8::Local<v8::String> handler_name = v8::String::NewFromUtf8(isolate_, "handleRequest").ToLocalChecked();
        v8::Local<v8::Value> handler_val = context->Global()->Get(context, handler_name).ToLocalChecked();
        
        HttpResponse response;
        
        if (handler_val->IsFunction()) {
            v8::Local<v8::Function> handler = v8::Local<v8::Function>::Cast(handler_val);
            v8::Local<v8::Value> args[] = { req_obj, res_obj };
            
            v8::TryCatch TryCatch(isolate_);
            handler->Call(context, context->Global(), 2, args).ToLocalChecked();
            
            if (TryCatch.HasCaught()) {
                v8::String::Utf8Value error(isolate_, TryCatch.Exception());
                std::cerr << "Handler error: " << *error << std::endl;
                response.status_code = 500;
                response.body = "Internal Server Error";
            } else {
                // Extract response
                v8::Local<v8::Value> status_val = res_obj->Get(context, 
                    v8::String::NewFromUtf8(isolate_, "statusCode").ToLocalChecked()).ToLocalChecked();
                response.status_code = status_val->Int32Value(context).FromJust();
                
                v8::Local<v8::Value> body_val = res_obj->Get(context,
                    v8::String::NewFromUtf8(isolate_, "body").ToLocalChecked()).ToLocalChecked();
                v8::String::Utf8Value BodyStr(isolate_, body_val);
                response.body = *BodyStr;
                
                // Extract headers
                v8::Local<v8::Value> headers_val = res_obj->Get(context,
                    v8::String::NewFromUtf8(isolate_, "headers").ToLocalChecked()).ToLocalChecked();
                if (headers_val->IsObject()) {
                    v8::Local<v8::Object> headers_obj = headers_val.As<v8::Object>();
                    v8::Local<v8::Array> keys = headers_obj->GetPropertyNames(context).ToLocalChecked();
                    for (uint32_t i = 0; i < keys->Length(); i++) {
                        v8::Local<v8::Value> key = keys->Get(context, i).ToLocalChecked();
                        v8::Local<v8::Value> value = headers_obj->Get(context, key).ToLocalChecked();
                        v8::String::Utf8Value KeyStr(isolate_, key);
                        v8::String::Utf8Value ValueStr(isolate_, value);
                        response.headers[*KeyStr] = *ValueStr;
                    }
                }
            }
        } else {
            // Default handler
            response.status_code = 404;
            response.body = "Not Found";
        }
        
        return response;
    }
    
    void start(int port) {
        running_ = true;
        server_thread_ = std::thread([this, port]() {
            std::cout << "V8 Web Server starting on port " << port << std::endl;
            
            // Simulate simple HTTP server
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                // Simulate incoming request
                HttpRequest request;
                request.method = "GET";
                request.path = "/api/test";
                request.headers["User-Agent"] = "V8-Test-Client";
                request.headers["Content-Type"] = "application/json";
                
                HttpResponse response = handleRequest(request);
                
                std::cout << "Request: " << request.method << " " << request.path << std::endl;
                std::cout << "Response: " << response.status_code << " - " << response.body << std::endl;
                
                // For demo purposes, only process one request
                break;
            }
        });
    }
    
    void stop() {
        running_ = false;
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
};

int main() {
    V8WebServer server;
    
    // Create a sample JavaScript request handler
    std::ofstream ScriptFile("request_handler.js");
    ScriptFile << R"(
        function handleRequest(req, res) {
            console.log('Handling request:', req.method, req.path);
            
            if (req.path === '/api/test') {
                res.json({
                    message: 'Hello from V8!',
                    timestamp: Date.now(),
                    method: req.method,
                    path: req.path
                });
            } else if (req.path === '/api/health') {
                res.json({
                    status: 'OK',
                    uptime: process.uptime ? process.uptime() : 'N/A'
                });
            } else {
                res.status(404).send('Not Found');
            }
        }
        
        console.log('Request handler loaded');
    )";
    ScriptFile.close();
    
    // Load the JavaScript handler
    server.loadScript("request_handler.js");
    
    // Start the server
    server.start(8080);
    
    // Wait for server to process request
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Stop the server
    server.stop();
    
    // Clean up
    std::remove("request_handler.js");
    
    return 0;
}