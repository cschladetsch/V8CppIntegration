#ifndef V8_COMPAT_H
#define V8_COMPAT_H

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>

namespace v8_compat {

// Platform creation wrapper - defined in Source/v8_platform_compat.cpp
std::unique_ptr<v8::Platform> CreateDefaultPlatform(int thread_pool_size = 0);

// ScriptOrigin creation wrapper to handle API differences between V8 versions
inline v8::ScriptOrigin CreateScriptOrigin(
    v8::Isolate* isolate,
    v8::Local<v8::Value> resource_name,
    int line_offset = 0,
    int column_offset = 0,
    bool is_shared_cross_origin = false,
    int script_id = -1,
    v8::Local<v8::Value> source_map_url = v8::Local<v8::Value>(),
    bool is_opaque = false,
    bool is_wasm = false,
    bool is_module = false) {
    
#if V8_MAJOR_VERSION >= 14
    // V8 14+ API - no isolate parameter, host_defined_options as last parameter
    return v8::ScriptOrigin(
        resource_name,
        line_offset,
        column_offset,
        is_shared_cross_origin,
        script_id,
        source_map_url,
        is_opaque,
        is_wasm,
        is_module
        // host_defined_options parameter is optional and defaults to Local<Data>()
    );
#elif V8_MAJOR_VERSION >= 11
    // V8 11-13 API - no isolate parameter  
    return v8::ScriptOrigin(
        resource_name,
        line_offset,
        column_offset,
        is_shared_cross_origin,
        script_id,
        source_map_url,
        is_opaque,
        is_wasm,
        is_module
    );
#elif defined(USE_SYSTEM_V8) && !defined(V8_MAJOR_VERSION)
    // System V8 - isolate as first parameter
    return v8::ScriptOrigin(
        isolate,
        resource_name,
        line_offset,
        column_offset,
        is_shared_cross_origin,
        script_id,
        source_map_url,
        is_opaque,
        is_wasm,
        is_module
    );
#else
    // Older V8 API (pre-v11) - isolate as first parameter
    return v8::ScriptOrigin(
        isolate,
        resource_name,
        line_offset,
        column_offset,
        is_shared_cross_origin,
        script_id,
        source_map_url,
        is_opaque,
        is_wasm,
        is_module
    );
#endif
}

// Simplified ScriptOrigin creation for common cases
inline v8::ScriptOrigin CreateScriptOrigin(
    v8::Isolate* isolate,
    const std::string& filename,
    bool is_module = false) {
    
    v8::Local<v8::String> name = v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked();
    return CreateScriptOrigin(isolate, name, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, is_module);
}

// TryCatch wrapper for consistent error handling
class TryCatch {
public:
    explicit TryCatch(v8::Isolate* isolate) : try_catch_(isolate) {}
    
    bool HasCaught() const { return try_catch_.HasCaught(); }
    
    v8::Local<v8::Value> Exception() { return try_catch_.Exception(); }
    
    v8::Local<v8::Message> Message() { return try_catch_.Message(); }
    
    v8::MaybeLocal<v8::Value> StackTrace(v8::Local<v8::Context> context) {
        return try_catch_.StackTrace(context);
    }
    
    void Reset() { try_catch_.Reset(); }
    
    std::string GetExceptionString(v8::Isolate* isolate) {
        if (!HasCaught()) return "";
        
        v8::String::Utf8Value exception(isolate, Exception());
        return *exception ? *exception : "Unknown exception";
    }
    
    std::string GetDetailedError(v8::Isolate* isolate, v8::Local<v8::Context> context) {
        if (!HasCaught()) return "";
        
        std::string error = GetExceptionString(isolate);
        
        v8::Local<v8::Message> message = Message();
        if (!message.IsEmpty()) {
            v8::String::Utf8Value filename(isolate, message->GetScriptResourceName());
            int line = message->GetLineNumber(context).FromMaybe(0);
            
            error += " at ";
            error += *filename ? *filename : "unknown";
            error += ":";
            error += std::to_string(line);
        }
        
        return error;
    }
    
private:
    v8::TryCatch try_catch_;
};

// Context creation with default settings
inline v8::Local<v8::Context> CreateContext(
    v8::Isolate* isolate,
    v8::Local<v8::ObjectTemplate> global_template = v8::Local<v8::ObjectTemplate>()) {
    
    if (global_template.IsEmpty()) {
        return v8::Context::New(isolate);
    }
    return v8::Context::New(isolate, nullptr, global_template);
}

// String conversion helpers
inline std::string ToStdString(v8::Isolate* isolate, v8::Local<v8::Value> value) {
    if (value.IsEmpty()) return "";
    v8::String::Utf8Value utf8(isolate, value);
    return *utf8 ? *utf8 : "";
}

inline v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const std::string& str) {
    return v8::String::NewFromUtf8(isolate, str.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
}

// Value conversion helpers
inline v8::Local<v8::Value> ToV8Value(v8::Isolate* isolate, bool value) {
    return v8::Boolean::New(isolate, value);
}

inline v8::Local<v8::Value> ToV8Value(v8::Isolate* isolate, int32_t value) {
    return v8::Integer::New(isolate, value);
}

inline v8::Local<v8::Value> ToV8Value(v8::Isolate* isolate, double value) {
    return v8::Number::New(isolate, value);
}

inline v8::Local<v8::Value> ToV8Value(v8::Isolate* isolate, const std::string& value) {
    return ToV8String(isolate, value);
}

// Object property helpers
inline bool SetProperty(
    v8::Local<v8::Context> context,
    v8::Local<v8::Object> object,
    const std::string& key,
    v8::Local<v8::Value> value) {
    
    v8::Isolate* isolate = context->GetIsolate();
    return object->Set(context, ToV8String(isolate, key), value).FromMaybe(false);
}

inline v8::MaybeLocal<v8::Value> GetProperty(
    v8::Local<v8::Context> context,
    v8::Local<v8::Object> object,
    const std::string& key) {
    
    v8::Isolate* isolate = context->GetIsolate();
    return object->Get(context, ToV8String(isolate, key));
}

// Function creation helper
template<typename Callback>
inline v8::Local<v8::Function> CreateFunction(
    v8::Local<v8::Context> context,
    Callback callback,
    const std::string& name = "") {
    
    v8::Isolate* isolate = context->GetIsolate();
    v8::Local<v8::FunctionTemplate> ft = v8::FunctionTemplate::New(isolate, callback);
    
    if (!name.empty()) {
        ft->SetClassName(ToV8String(isolate, name));
    }
    
    return ft->GetFunction(context).ToLocalChecked();
}

// Promise helpers
inline v8::Local<v8::Promise::Resolver> CreatePromiseResolver(v8::Local<v8::Context> context) {
    return v8::Promise::Resolver::New(context).ToLocalChecked();
}

inline void ResolvePromise(
    v8::Local<v8::Context> context,
    v8::Local<v8::Promise::Resolver> resolver,
    v8::Local<v8::Value> value) {
    
    resolver->Resolve(context, value).Check();
}

inline void RejectPromise(
    v8::Local<v8::Context> context,
    v8::Local<v8::Promise::Resolver> resolver,
    v8::Local<v8::Value> reason) {
    
    resolver->Reject(context, reason).Check();
}

// Array helpers
inline v8::Local<v8::Array> CreateArray(v8::Isolate* isolate, size_t length = 0) {
    return v8::Array::New(isolate, static_cast<int>(length));
}

inline bool SetArrayElement(
    v8::Local<v8::Context> context,
    v8::Local<v8::Array> array,
    uint32_t index,
    v8::Local<v8::Value> value) {
    
    return array->Set(context, index, value).FromMaybe(false);
}

// Module compilation helper
inline v8::MaybeLocal<v8::Module> CompileModule(
    v8::Isolate* isolate,
    const std::string& source,
    const std::string& filename) {
    
    v8::Local<v8::String> source_str = ToV8String(isolate, source);
    v8::ScriptOrigin origin = CreateScriptOrigin(isolate, filename, true);
    v8::ScriptCompiler::Source source_obj(source_str, origin);
    
    return v8::ScriptCompiler::CompileModule(isolate, &source_obj);
}

// Script compilation and execution helper
inline v8::MaybeLocal<v8::Value> CompileAndRun(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    const std::string& source,
    const std::string& filename = "<anonymous>") {
    
    TryCatch try_catch(isolate);
    
    v8::Local<v8::String> source_str = ToV8String(isolate, source);
    v8::ScriptOrigin origin = CreateScriptOrigin(isolate, filename);
    
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, source_str, &origin).ToLocal(&script)) {
        return v8::MaybeLocal<v8::Value>();
    }
    
    return script->Run(context);
}

// JSON parsing helpers
inline v8::MaybeLocal<v8::Value> ParseJSON(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    const std::string& json) {
    
    v8::Local<v8::String> json_str = ToV8String(isolate, json);
    return v8::JSON::Parse(context, json_str);
}

inline v8::MaybeLocal<v8::String> StringifyJSON(
    v8::Local<v8::Context> context,
    v8::Local<v8::Value> value) {
    
    return v8::JSON::Stringify(context, value);
}

// V8 version detection helpers
inline int GetV8MajorVersion() {
    return V8_MAJOR_VERSION;
}

inline int GetV8MinorVersion() {
    return V8_MINOR_VERSION;
}

inline bool IsV8VersionAtLeast(int major, int minor = 0) {
    return (V8_MAJOR_VERSION > major) || 
           (V8_MAJOR_VERSION == major && V8_MINOR_VERSION >= minor);
}

} // namespace v8_compat

#endif // V8_COMPAT_H