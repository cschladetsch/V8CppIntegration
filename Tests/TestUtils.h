#pragma once

#include <v8.h>
#include <libplatform/libplatform.h>

namespace v8_test {

// RAII helper class that sets up V8 environment for tests
class V8TestEnvironment {
public:
    v8::Isolate* isolate;
    
private:
    v8::Isolate::Scope isolate_scope_;
    v8::HandleScope handle_scope_;
    
public:
    v8::Local<v8::Context> context;
    
private:
    v8::Context::Scope context_scope_;

public:
    explicit V8TestEnvironment(v8::Isolate* iso) 
        : isolate(iso),
          isolate_scope_(isolate),
          handle_scope_(isolate),
          context(v8::Context::New(isolate)),
          context_scope_(context) {}

    // Disable copy and move
    V8TestEnvironment(const V8TestEnvironment&) = delete;
    V8TestEnvironment& operator=(const V8TestEnvironment&) = delete;
    V8TestEnvironment(V8TestEnvironment&&) = delete;
    V8TestEnvironment& operator=(V8TestEnvironment&&) = delete;
};

} // namespace v8_test