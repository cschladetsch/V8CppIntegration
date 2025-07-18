#include <v8.h>
#include <libplatform/libplatform.h>
#include <iostream>
#include <memory>
#include "V8ModuleSystem.h"

using namespace v8;
using namespace v8mod;

void MathAdd(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numeric arguments required").ToLocalChecked()));
        return;
    }
    
    double a = args[0].As<Number>()->Value();
    double b = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, a + b));
}

void MathSubtract(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numeric arguments required").ToLocalChecked()));
        return;
    }
    
    double a = args[0].As<Number>()->Value();
    double b = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, a - b));
}

void MathPower(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numeric arguments required").ToLocalChecked()));
        return;
    }
    
    double base = args[0].As<Number>()->Value();
    double exponent = args[1].As<Number>()->Value();
    
    args.GetReturnValue().Set(Number::New(isolate, std::pow(base, exponent)));
}

DEFINE_V8_MODULE(MathModule, "math", "1.0.0");

void MathModule::RegisterFunctions(V8Module& module) {
    module.AddFunction("add", MathAdd, "Add two numbers")
          .AddFunction("subtract", MathSubtract, "Subtract two numbers")
          .AddFunction("power", MathPower, "Raise a number to a power");
}

void StringReverse(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "String argument required").ToLocalChecked()));
        return;
    }
    
    String::Utf8Value str(isolate, args[0]);
    std::string s(*str);
    std::reverse(s.begin(), s.end());
    
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, s.c_str()).ToLocalChecked());
}

void StringRepeat(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsInt32()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "String and number arguments required").ToLocalChecked()));
        return;
    }
    
    String::Utf8Value str(isolate, args[0]);
    int count = args[1].As<Int32>()->Value();
    
    if (count < 0) {
        isolate->ThrowException(Exception::RangeError(
            String::NewFromUtf8(isolate, "Count must be non-negative").ToLocalChecked()));
        return;
    }
    
    std::string result;
    for (int i = 0; i < count; i++) {
        result += *str;
    }
    
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, result.c_str()).ToLocalChecked());
}

DEFINE_V8_MODULE(StringModule, "string", "1.0.0");

void StringModule::RegisterFunctions(V8Module& module) {
    module.AddFunction("reverse", StringReverse, "Reverse a string")
          .AddFunction("repeat", StringRepeat, "Repeat a string n times");
}

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
        
        ModuleRegistry<MathModule>::GetModule().Install(isolate, context);
        ModuleRegistry<StringModule>::GetModule().Install(isolate, context);
        
        const char* script_source = R"(
            console.log('Math module:');
            console.log('math.add(5, 3) =', math.add(5, 3));
            console.log('math.subtract(10, 4) =', math.subtract(10, 4));
            console.log('math.power(2, 8) =', math.power(2, 8));
            
            console.log('\nString module:');
            console.log('string.reverse("hello") =', string.reverse("hello"));
            console.log('string.repeat("abc", 3) =', string.repeat("abc", 3));
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