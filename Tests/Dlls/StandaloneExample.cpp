#include <v8.h>
#include <libplatform/libplatform.h>
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <map>

using namespace v8;

// Simple function registry that avoids DLL loading
class StaticFunctionRegistry {
public:
    using FunctionEntry = std::pair<std::string, FunctionCallback>;
    
    static StaticFunctionRegistry& Instance() {
        static StaticFunctionRegistry instance;
        return instance;
    }
    
    void Register(const std::string& name, FunctionCallback callback) {
        functions_.push_back({name, callback});
    }
    
    void InstallAll(Isolate* isolate, Local<Context> context) {
        Local<Object> global = context->Global();
        
        for (const auto& [name, callback] : functions_) {
            global->Set(context,
                String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
                Function::New(context, callback).ToLocalChecked()
            ).FromJust();
        }
    }
    
private:
    std::vector<FunctionEntry> functions_;
};

// Macro for easy registration
#define REGISTER_V8_FUNCTION(name) \
    namespace { \
        struct Register_##name { \
            Register_##name() { \
                StaticFunctionRegistry::Instance().Register(#name, name); \
            } \
        } register_##name; \
    }

// Example functions
void add(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numbers required").ToLocalChecked()));
        return;
    }
    
    double a = args[0].As<Number>()->Value();
    double b = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, a + b));
}
REGISTER_V8_FUNCTION(add)

void multiply(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numbers required").ToLocalChecked()));
        return;
    }
    
    double a = args[0].As<Number>()->Value();
    double b = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, a * b));
}
REGISTER_V8_FUNCTION(multiply)

void greet(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "String required").ToLocalChecked()));
        return;
    }
    
    String::Utf8Value name(isolate, args[0]);
    std::string greeting = "Hello, " + std::string(*name) + "!";
    
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, greeting.c_str()).ToLocalChecked());
}
REGISTER_V8_FUNCTION(greet)

int main() {
    // Initialize V8
    std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    
    // Create isolate
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Install all registered functions
        StaticFunctionRegistry::Instance().InstallAll(isolate, context);
        
        // Example usage
        const char* script_source = R"(
            console.log('Testing static function registration:');
            console.log('add(5, 3) =', add(5, 3));
            console.log('multiply(4, 7) =', multiply(4, 7));
            console.log('greet("World") =', greet("World"));
            
            // Show that we don't need DLL loading
            console.log('\nAll functions are compiled directly into the binary.');
            console.log('No DLL loading or registerFunc required!');
        )";
        
        // Compile and run script
        Local<String> source = String::NewFromUtf8(isolate, script_source).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        
        Local<Value> result = script->Run(context).ToLocalChecked();
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}

/*
To compile this standalone example:
g++ -std=c++17 StandaloneExample.cpp -o standalone_example \
    -I/path/to/v8/include \
    -L/path/to/v8/lib \
    -lv8 -lv8_libplatform -lv8_libbase \
    -pthread -ldl
*/