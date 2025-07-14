#include <benchmark/benchmark.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <chrono>

class V8PerformanceFixture : public benchmark::Fixture {
public:
    static std::unique_ptr<v8::Platform> platform;
    static bool v8_initialized;
    
    void SetUp(const ::benchmark::State& state) override {
        if (!v8_initialized) {
            v8::V8::InitializeICUDefaultLocation(".");
            v8::V8::InitializeExternalStartupData(".");
            platform = v8::platform::NewDefaultPlatform();
            v8::V8::InitializePlatform(platform.get());
            v8::V8::Initialize();
            v8_initialized = true;
        }
        
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = v8::Isolate::New(create_params);
        
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        context = v8::Context::New(isolate);
    }
    
    void TearDown(const ::benchmark::State& state) override {
        context.Reset();
        isolate->Dispose();
    }
    
protected:
    v8::Isolate* isolate;
    v8::Global<v8::Context> context;
};

std::unique_ptr<v8::Platform> V8PerformanceFixture::platform;
bool V8PerformanceFixture::v8_initialized = false;

// Basic JavaScript execution performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, SimpleExecution)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = "2 + 3";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, SimpleExecution)->Iterations(100000);

// Function call performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, FunctionCall)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = "function test(a, b) { return a + b; }";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
    script->Run(ctx).ToLocalChecked();
    
    v8::Local<v8::String> func_name = v8::String::NewFromUtf8(isolate, "test").ToLocalChecked();
    v8::Local<v8::Value> func_val = ctx->Global()->Get(ctx, func_name).ToLocalChecked();
    v8::Local<v8::Function> func = v8::Local<v8::Function>::Cast(func_val);
    
    v8::Local<v8::Value> args[] = {
        v8::Number::New(isolate, 10),
        v8::Number::New(isolate, 20)
    };
    
    for (auto _ : state) {
        v8::Local<v8::Value> result = func->Call(ctx, ctx->Global(), 2, args).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, FunctionCall)->Iterations(50000);

// Object creation performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, ObjectCreation)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    for (auto _ : state) {
        v8::Local<v8::Object> obj = v8::Object::New(isolate);
        v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, "test").ToLocalChecked();
        v8::Local<v8::String> value = v8::String::NewFromUtf8(isolate, "value").ToLocalChecked();
        obj->Set(ctx, key, value).FromJust();
        benchmark::DoNotOptimize(obj);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, ObjectCreation)->Iterations(10000);

// Array operations performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, ArrayOperations)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = "let arr = []; for(let i = 0; i < 1000; i++) arr.push(i); arr.length";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, ArrayOperations)->Iterations(1000);

// JSON parsing performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, JSONParsing)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    std::string json_data = R"({
        "name": "test",
        "value": 123,
        "array": [1, 2, 3, 4, 5],
        "nested": {
            "property": "value",
            "number": 42.5
        }
    })";
    
    std::string source = "JSON.parse('" + json_data + "')";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source.c_str()).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, JSONParsing)->Iterations(10000);

// String operations performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, StringOperations)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = "let str = 'Hello'; str + ' World'; str.length; str.substring(0, 5)";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, StringOperations)->Iterations(50000);

// C++ to JavaScript binding performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, CppToJSBinding)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    auto callback = [](const v8::FunctionCallbackInfo<v8::Value>& args) {
        if (args.Length() >= 2) {
            int32_t a = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            int32_t b = args[1]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            args.GetReturnValue().Set(v8::Number::New(args.GetIsolate(), a + b));
        }
    };
    
    v8::Local<v8::Function> func = v8::Function::New(ctx, callback).ToLocalChecked();
    v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, "cppAdd").ToLocalChecked();
    ctx->Global()->Set(ctx, name, func).FromJust();
    
    const char* source = "cppAdd(10, 20)";
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, CppToJSBinding)->Iterations(10000);

// Memory allocation performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, MemoryAllocation)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    size_t allocated_bytes = 0;
    
    for (auto _ : state) {
        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, 1024);
        allocated_bytes += buffer->ByteLength();
        benchmark::DoNotOptimize(buffer);
    }
    
    state.SetBytesProcessed(allocated_bytes);
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, MemoryAllocation)->Iterations(1000);

// Garbage collection performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, GarbageCollection)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = R"(
        let objects = [];
        for (let i = 0; i < 10000; i++) {
            objects.push({ id: i, data: new Array(100).fill(i) });
        }
        objects = null;
    )";
    
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        script->Run(ctx).ToLocalChecked();
        
        // Force garbage collection
        isolate->LowMemoryNotification();
        benchmark::DoNotOptimize(script);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, GarbageCollection)->Iterations(100);

// Regular expression performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, RegexOperations)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = R"(
        let text = 'The quick brown fox jumps over the lazy dog';
        let regex = /\b\w+\b/g;
        text.match(regex);
    )";
    
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, RegexOperations)->Iterations(10000);

// Promise performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, PromiseOperations)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = R"(
        new Promise((resolve) => {
            resolve(42);
        }).then(value => value * 2);
    )";
    
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, PromiseOperations)->Iterations(1000);

// Map/Set performance
BENCHMARK_DEFINE_F(V8PerformanceFixture, MapSetOperations)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = R"(
        let map = new Map();
        let set = new Set();
        for (let i = 0; i < 1000; i++) {
            map.set(i, i * 2);
            set.add(i);
        }
        map.size + set.size;
    )";
    
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, MapSetOperations)->Iterations(1000);

// Stress test with complex operations
BENCHMARK_DEFINE_F(V8PerformanceFixture, StressTest)(benchmark::State& state) {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> ctx = v8::Local<v8::Context>::New(isolate, context);
    v8::Context::Scope context_scope(ctx);
    
    const char* source = R"(
        function fibonacci(n) {
            if (n <= 1) return n;
            return fibonacci(n - 1) + fibonacci(n - 2);
        }
        
        function processData(data) {
            return data.map(x => ({
                original: x,
                squared: x * x,
                fibonacci: fibonacci(x % 10)
            })).filter(item => item.squared > 100);
        }
        
        let data = Array.from({length: 100}, (_, i) => i);
        processData(data);
    )";
    
    v8::Local<v8::String> src = v8::String::NewFromUtf8(isolate, source).ToLocalChecked();
    
    for (auto _ : state) {
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, src).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(ctx).ToLocalChecked();
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(V8PerformanceFixture, StressTest)->Iterations(10);

// Custom main function to add additional reporting
int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    
    // Add custom output formats
    benchmark::RegisterBenchmark("V8_Version", [](benchmark::State& state) {
        for (auto _ : state) {
            std::string version = v8::V8::GetVersion();
            benchmark::DoNotOptimize(version);
        }
    });
    
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    
    return 0;
}