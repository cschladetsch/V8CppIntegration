#include "V8ConsoleCompletion.h"
#include "V8Console.h"
#include <algorithm>
#include <sstream>
#include <cstring>

#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

// Static member definition
V8ConsoleCompletion* V8ConsoleCompletion::instance_ = nullptr;

V8ConsoleCompletion::V8ConsoleCompletion(V8Console* console) 
    : console_(console) {
}

v8::Isolate* V8ConsoleCompletion::GetIsolate() const {
    return console_->GetIsolate();
}

v8::Local<v8::Context> V8ConsoleCompletion::GetContext() const {
    return console_->GetContext();
}

#ifndef NO_READLINE
char** V8ConsoleCompletion::CompletionGenerator(const char* text, int start, int end) {
    if (!instance_) return nullptr;
    
    // Get the current line buffer
    std::string line(rl_line_buffer);
    
    // Check if we're in JavaScript mode (starts with &)
    if (!line.empty() && line[0] == '&') {
        // Remove the & prefix for JavaScript completion
        std::string jsLine = line.substr(1);
        int jsStart = start > 0 ? start - 1 : 0;
        int jsEnd = end > 0 ? end - 1 : 0;
        
        auto completions = instance_->GetCompletions(text, jsStart, jsEnd);
        if (completions.empty()) return nullptr;
        
        // Convert to readline format
        char** matches = static_cast<char**>(malloc((completions.size() + 2) * sizeof(char*)));
        matches[0] = strdup(text);
        
        for (size_t i = 0; i < completions.size(); ++i) {
            matches[i + 1] = strdup(completions[i].c_str());
        }
        matches[completions.size() + 1] = nullptr;
        
        return matches;
    }
    
    // For shell commands, use default filename completion
    return nullptr;
}

std::vector<std::string> V8ConsoleCompletion::GetCompletions(const std::string& text, int start, int end) {
    std::vector<std::string> completions;
    
    auto* isolate = GetIsolate();
    if (!isolate) return completions;
    
    // Get the current line up to the cursor
    std::string line = rl_line_buffer;
    if (line.length() > 0 && line[0] == '&') {
        line = line.substr(1, start);
    } else {
        line = line.substr(0, start);
    }
    
    // Find the object path (e.g., "console." or "myObj.nested.")
    size_t lastDot = line.rfind('.');
    std::string objectPath;
    std::string prefix;
    
    if (lastDot != std::string::npos) {
        objectPath = line.substr(0, lastDot);
        prefix = line.substr(lastDot + 1);
    } else {
        // Global completions
        objectPath = "";
        prefix = text;
    }
    
    // Get properties from the object
    auto properties = GetObjectProperties(objectPath);
    
    // Filter by prefix
    for (const auto& prop : properties) {
        if (prop.find(prefix) == 0) {
            completions.push_back(prop);
        }
    }
    
    return completions;
}

std::vector<std::string> V8ConsoleCompletion::GetObjectProperties(const std::string& objectPath) {
    std::vector<std::string> properties;
    
    auto* isolate = GetIsolate();
    if (!isolate) return properties;
    
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope context_scope(context);
    
    v8::TryCatch try_catch(isolate);
    
    // Start with the global object
    v8::Local<v8::Object> obj = context->Global();
    
    // If there's an object path, traverse it
    if (!objectPath.empty()) {
        // Split the path by dots
        std::vector<std::string> parts;
        std::stringstream ss(objectPath);
        std::string part;
        while (std::getline(ss, part, '.')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        
        // Traverse the object hierarchy
        for (const auto& part : parts) {
            v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate, part.c_str()).ToLocalChecked();
            v8::Local<v8::Value> value;
            
            if (!obj->Get(context, key).ToLocal(&value) || !value->IsObject()) {
                return properties; // Object not found or not an object
            }
            
            obj = value.As<v8::Object>();
        }
    }
    
    // Get own property names
    v8::Local<v8::Array> propertyNames;
    if (!obj->GetOwnPropertyNames(context).ToLocal(&propertyNames)) {
        return properties;
    }
    
    // Also get properties from prototype chain for built-in objects
    v8::Local<v8::Array> prototypeNames;
    if (obj->GetPropertyNames(context).ToLocal(&prototypeNames)) {
        // Merge prototype properties
        for (uint32_t i = 0; i < prototypeNames->Length(); ++i) {
            v8::Local<v8::Value> name;
            if (prototypeNames->Get(context, i).ToLocal(&name) && name->IsString()) {
                v8::String::Utf8Value utf8Name(isolate, name);
                std::string propName(*utf8Name);
                
                // Check if this is a function/method
                v8::Local<v8::Value> propValue;
                if (obj->Get(context, name).ToLocal(&propValue)) {
                    if (propValue->IsFunction()) {
                        propName += "(";  // Add parenthesis hint for functions
                    }
                    properties.push_back(propName);
                }
            }
        }
    } else {
        // Fallback to just own properties
        for (uint32_t i = 0; i < propertyNames->Length(); ++i) {
            v8::Local<v8::Value> name;
            if (propertyNames->Get(context, i).ToLocal(&name) && name->IsString()) {
                v8::String::Utf8Value utf8Name(isolate, name);
                std::string propName(*utf8Name);
                
                // Check if this is a function
                v8::Local<v8::Value> propValue;
                if (obj->Get(context, name).ToLocal(&propValue) && propValue->IsFunction()) {
                    propName += "(";  // Add parenthesis hint for functions
                }
                properties.push_back(propName);
            }
        }
    }
    
    // Remove duplicates and sort
    std::sort(properties.begin(), properties.end());
    properties.erase(std::unique(properties.begin(), properties.end()), properties.end());
    
    return properties;
}
#else
// Stub implementations when readline is not available
char** V8ConsoleCompletion::CompletionGenerator(const char*, int, int) { return nullptr; }
std::vector<std::string> V8ConsoleCompletion::GetCompletions(const std::string&, int, int) { return {}; }
std::vector<std::string> V8ConsoleCompletion::GetObjectProperties(const std::string&) { return {}; }
#endif