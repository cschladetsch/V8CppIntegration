#include <iostream>
#include <v8.h>
#include <libplatform/libplatform.h>

using namespace v8;

// C++ function that will be called from JavaScript
void CppPrint(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    if (args.Length() > 0) {
        String::Utf8Value str(isolate, args[0]);
        std::cout << "[C++] Received from JS: " << *str << std::endl;
    }
}

// C++ function that adds two numbers
void CppAdd(const FunctionCallbackInfo<Value>& args) {
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

int main(int argc, char* argv[]) {
    // Initialize V8
    V8::InitializeICUDefaultLocation(argv[0]);
    V8::InitializeExternalStartupData(argv[0]);
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
        
        // Get global object
        Local<Object> global = context->Global();
        
        // Register C++ functions to be callable from JavaScript
        global->Set(context,
            String::NewFromUtf8(isolate, "print").ToLocalChecked(),
            Function::New(context, CppPrint).ToLocalChecked()).Check();
            
        global->Set(context,
            String::NewFromUtf8(isolate, "add").ToLocalChecked(),
            Function::New(context, CppAdd).ToLocalChecked()).Check();
        
        std::cout << "\n=== V8 C++ Integration Example ===" << std::endl;
        std::cout << "\n1. Calling C++ from JavaScript:" << std::endl;
        
        // JavaScript code that calls our C++ functions
        const char* js_code = R"(
            print('Hello from JavaScript!');
            var result = add(10, 32);
            print('10 + 32 = ' + result);
            
            // Create an object to return
            var data = {
                message: 'Calculation complete',
                result: result
            };
            JSON.stringify(data);
        )";
        
        // Compile and run JavaScript
        Local<String> source = String::NewFromUtf8(isolate, js_code).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).ToLocalChecked();
        
        // Get the return value from JavaScript
        String::Utf8Value utf8(isolate, result);
        std::cout << "\n[C++] JavaScript returned: " << *utf8 << std::endl;
        
        // Example 2: Call JavaScript function from C++
        std::cout << "\n2. Calling JavaScript from C++:" << std::endl;
        
        const char* js_func = R"(
            function multiply(a, b) {
                print('JS multiply called with: ' + a + ' and ' + b);
                return a * b;
            }
        )";
        
        // Define the JavaScript function
        Local<Script> func_script = Script::Compile(context, String::NewFromUtf8(isolate, js_func).ToLocalChecked()).ToLocalChecked();
        func_script->Run(context).ToLocalChecked();
        
        // Get the function
        Local<Value> multiply_val = global->Get(context,
            String::NewFromUtf8(isolate, "multiply").ToLocalChecked()).ToLocalChecked();
        Local<Function> multiply_func = Local<Function>::Cast(multiply_val);
        
        // Call it with arguments
        Local<Value> args[] = {
            Number::New(isolate, 7),
            Number::New(isolate, 6)
        };
        Local<Value> js_result = multiply_func->Call(context, global, 2, args).ToLocalChecked();
        
        double product = js_result.As<Number>()->Value();
        std::cout << "[C++] Result from JavaScript: " << product << std::endl;
        
        std::cout << "\n=== Demo complete! ===" << std::endl;
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}