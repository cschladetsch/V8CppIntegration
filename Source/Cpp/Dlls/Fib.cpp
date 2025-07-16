#include <v8.h>
#include <iostream>
#include <vector>

using namespace v8;

// Calculate the sum of the first N Fibonacci numbers
// fib(0) = 0, fib(1) = 1, fib(2) = 1, fib(3) = 2, ...
// Returns the sum: fib(0) + fib(1) + ... + fib(N-1)
long long calculateFibSum(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 0;  // Only fib(0) = 0
    if (n == 2) return 1;  // fib(0) + fib(1) = 0 + 1 = 1
    
    long long prev2 = 0;  // fib(0)
    long long prev1 = 1;  // fib(1)
    long long sum = 1;    // sum of fib(0) + fib(1)
    
    for (int i = 2; i < n; i++) {
        long long current = prev1 + prev2;
        sum += current;
        prev2 = prev1;
        prev1 = current;
    }
    
    return sum;
}

// V8 wrapper function that can be called from JavaScript
void Fib(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    // Check if we have the right number of arguments
    if (args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Wrong number of arguments. Expected 1 argument.").ToLocalChecked()));
        return;
    }
    
    // Check if the argument is a number
    if (!args[0]->IsNumber()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Argument must be a number").ToLocalChecked()));
        return;
    }
    
    // Get the number value
    int n = args[0]->Int32Value(isolate->GetCurrentContext()).FromMaybe(0);
    
    // Validate input
    if (n < 0) {
        isolate->ThrowException(Exception::RangeError(
            String::NewFromUtf8(isolate, "Argument must be non-negative").ToLocalChecked()));
        return;
    }
    
    // Calculate the sum
    long long result = calculateFibSum(n);
    
    // Return the result
    args.GetReturnValue().Set(Number::New(isolate, static_cast<double>(result)));
}

// Export the initialization function that v8console expects
extern "C" {
    void RegisterV8Functions(Isolate* isolate, Local<Context> context) {
        // Get the global object
        Local<Object> global = context->Global();
        
        // Add the fib function to the global object
        global->Set(context,
            String::NewFromUtf8(isolate, "fib").ToLocalChecked(),
            Function::New(context, Fib).ToLocalChecked()
        ).FromJust();
        
        std::cout << "Fibonacci module loaded. Use fib(n) to calculate sum of first n Fibonacci numbers." << std::endl;
    }
}