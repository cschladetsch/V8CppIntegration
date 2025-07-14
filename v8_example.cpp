#include <iostream>
#include <string>
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace v8;

// Global isolate pointer for callbacks
Isolate* g_isolate = nullptr;

// C++ function that will be called from JavaScript
void CppFunction(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    // Check arguments
    if (args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong number of arguments").ToLocalChecked()));
        return;
    }
    
    // Convert first argument to string
    String::Utf8Value str(isolate, args[0]);
    std::cout << "C++ function called from JS with: " << *str << std::endl;
    
    // Return a value back to JavaScript
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, 
        ("Hello from C++! You said: " + std::string(*str)).c_str()).ToLocalChecked());
}

// Another C++ function demonstrating number operations
void AddNumbers(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Two numbers required").ToLocalChecked()));
        return;
    }
    
    double a = args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked();
    double b = args[1]->NumberValue(isolate->GetCurrentContext()).ToChecked();
    
    args.GetReturnValue().Set(Number::New(isolate, a + b));
}

// Function to call JavaScript from C++
void CallJavaScriptFunction(Isolate* isolate, Local<Context> context, 
                           const char* js_code, const char* function_name) {
    HandleScope handle_scope(isolate);
    Context::Scope context_scope(context);
    
    // Compile and run the JavaScript code
    Local<String> source = String::NewFromUtf8(isolate, js_code).ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    script->Run(context).ToLocalChecked();
    
    // Get the function from global scope
    Local<Object> global = context->Global();
    Local<Value> func_val = global->Get(context, 
        String::NewFromUtf8(isolate, function_name).ToLocalChecked()).ToLocalChecked();
    
    if (func_val->IsFunction()) {
        Local<Function> func = Local<Function>::Cast(func_val);
        
        // Prepare arguments
        const int argc = 2;
        Local<Value> argv[argc] = {
            String::NewFromUtf8(isolate, "Hello from C++").ToLocalChecked(),
            Number::New(isolate, 42)
        };
        
        // Call the JavaScript function
        Local<Value> result = func->Call(context, global, argc, argv).ToLocalChecked();
        
        // Print the result
        String::Utf8Value utf8(isolate, result);
        std::cout << "JavaScript function returned: " << *utf8 << std::endl;
    }
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
    create_params.array_buffer_allocator = 
        ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    g_isolate = isolate;
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create a new context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Register C++ functions to be callable from JavaScript
        Local<Object> global = context->Global();
        global->Set(context,
            String::NewFromUtf8(isolate, "cppFunction").ToLocalChecked(),
            Function::New(context, CppFunction).ToLocalChecked()).Check();
        
        global->Set(context,
            String::NewFromUtf8(isolate, "addNumbers").ToLocalChecked(),
            Function::New(context, AddNumbers).ToLocalChecked()).Check();
        
        // Example 1: Call C++ from JavaScript
        std::cout << "\n=== Example 1: Calling C++ from JavaScript ===" << std::endl;
        const char* js_code1 = R"(
            // Call the C++ function
            var result = cppFunction('JavaScript calling C++!');
            console.log('Result from C++:', result);
            
            // Call the add function
            var sum = addNumbers(10, 32);
            console.log('Sum from C++:', sum);
        )";
        
        Local<String> source1 = String::NewFromUtf8(isolate, js_code1).ToLocalChecked();
        Local<Script> script1 = Script::Compile(context, source1).ToLocalChecked();
        script1->Run(context).ToLocalChecked();
        
        // Example 2: Call JavaScript from C++
        std::cout << "\n=== Example 2: Calling JavaScript from C++ ===" << std::endl;
        const char* js_code2 = R"(
            function processData(message, number) {
                return 'JS processed: ' + message + ' with number ' + (number * 2);
            }
        )";
        
        CallJavaScriptFunction(isolate, context, js_code2, "processData");
        
        // Example 3: Complex bidirectional communication
        std::cout << "\n=== Example 3: Bidirectional Communication ===" << std::endl;
        const char* js_code3 = R"(
            function jsCallback(value) {
                console.log('JS callback received:', value);
                // Call back to C++
                var result = cppFunction('Callback from JS with value: ' + value);
                return result;
            }
            
            // Simulate an async operation
            console.log('Starting JS operation...');
            var finalResult = jsCallback(100);
            console.log('Final result:', finalResult);
        )";
        
        Local<String> source3 = String::NewFromUtf8(isolate, js_code3).ToLocalChecked();
        Local<Script> script3 = Script::Compile(context, source3).ToLocalChecked();
        script3->Run(context).ToLocalChecked();
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}