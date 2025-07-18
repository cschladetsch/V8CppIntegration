#include <v8.h>
#include <libplatform/libplatform.h>
#include <iostream>
#include <memory>
#include "V8AutoRegister.h"

using namespace v8;
using namespace v8reg;

void Fibonacci(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsInt32()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong argument type").ToLocalChecked()));
        return;
    }
    
    int n = args[0].As<Int32>()->Value();
    
    if (n < 0) {
        isolate->ThrowException(Exception::RangeError(
            String::NewFromUtf8(isolate, "Argument must be non-negative").ToLocalChecked()));
        return;
    }
    
    if (n <= 1) {
        args.GetReturnValue().Set(n);
        return;
    }
    
    long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        long long temp = a + b;
        a = b;
        b = temp;
    }
    
    args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(b)));
}

void Multiply(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numeric arguments required").ToLocalChecked()));
        return;
    }
    
    double a = args[0].As<Number>()->Value();
    double b = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, a * b));
}

V8_REGISTER_FUNCTION(fibonacci, Fibonacci);
V8_REGISTER_FUNCTION(multiply, Multiply);

int main() {
    std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        FunctionRegistry::Instance().InstallAll(isolate, context);
        
        const char* script_source = R"(
            console.log('Fibonacci(10) =', fibonacci(10));
            console.log('Multiply(7, 6) =', multiply(7, 6));
            
            console.log('\nRegistered functions:');
            if (typeof fibonacci !== 'undefined') console.log('- fibonacci');
            if (typeof multiply !== 'undefined') console.log('- multiply');
        )";
        
        Local<String> source = String::NewFromUtf8(isolate, script_source).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        
        Local<Value> result = script->Run(context).ToLocalChecked();
    }
    
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}