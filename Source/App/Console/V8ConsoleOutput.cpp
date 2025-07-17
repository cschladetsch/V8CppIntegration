#include "V8Console.h"
#include <iostream>
#include <sstream>
#include <rang/rang.hpp>

void V8Console::PrintResult(v8::Local<v8::Value> value) {
    using namespace rang;
    
    // Check if the value might be JSON
    if (value->IsObject() || value->IsArray()) {
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = context_.Get(isolate_);
        v8::Context::Scope context_scope(context);
        
        // Try to stringify as JSON
        v8::Local<v8::String> jsonString;
        if (v8::JSON::Stringify(context, value).ToLocal(&jsonString)) {
            v8::String::Utf8Value utf8(isolate_, jsonString);
            std::string jsonStr(*utf8);
            
            // Pretty print the JSON
            PrettyPrintJSON(jsonStr);
            return;
        }
    }
    
    // For non-JSON values, print normally
    v8::String::Utf8Value utf8(isolate_, value);
    std::cout << fg::green << *utf8 << style::reset << std::endl;
}

void V8Console::PrettyPrintJSON(const std::string& json, int indent) {
    using namespace rang;
    
    bool inString = false;
    bool escaped = false;
    std::string indentStr(indent * 2, ' ');
    
    for (size_t i = 0; i < json.length(); ++i) {
        char c = json[i];
        
        if (!inString) {
            switch (c) {
                case '{':
                case '[':
                    std::cout << fg::yellow << c << style::reset;
                    if (i + 1 < json.length() && json[i + 1] != '}' && json[i + 1] != ']') {
                        std::cout << std::endl;
                        indent++;
                        indentStr = std::string(indent * 2, ' ');
                        std::cout << indentStr;
                    }
                    break;
                    
                case '}':
                case ']':
                    if (i > 0 && json[i - 1] != '{' && json[i - 1] != '[') {
                        indent--;
                        indentStr = std::string(indent * 2, ' ');
                        std::cout << std::endl << indentStr;
                    }
                    std::cout << fg::yellow << c << style::reset;
                    break;
                    
                case ',':
                    std::cout << c << std::endl << indentStr;
                    break;
                    
                case ':':
                    std::cout << fg::gray << ": " << style::reset;
                    break;
                    
                case '"':
                    std::cout << fg::green << c;
                    inString = true;
                    break;
                    
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    // Skip whitespace outside strings
                    break;
                    
                default:
                    // Numbers, true, false, null
                    if (std::isdigit(c) || c == '-' || c == '.') {
                        std::cout << fg::cyan;
                    } else if (json.substr(i, 4) == "true" || json.substr(i, 5) == "false") {
                        std::cout << fg::magenta;
                    } else if (json.substr(i, 4) == "null") {
                        std::cout << fg::red;
                    }
                    std::cout << c << style::reset;
                    break;
            }
        } else {
            // Inside a string
            if (escaped) {
                std::cout << c;
                escaped = false;
            } else if (c == '\\') {
                std::cout << c;
                escaped = true;
            } else if (c == '"') {
                std::cout << c << style::reset;
                inString = false;
            } else {
                std::cout << c;
            }
        }
    }
    
    std::cout << std::endl;
}

bool V8Console::IsJSON(v8::Local<v8::Value> value) {
    return value->IsObject() || value->IsArray();
}

void V8Console::ReportException(v8::TryCatch* tryCatch) {
    using namespace rang;
    
    v8::HandleScope handle_scope(isolate_);
    v8::String::Utf8Value exception(isolate_, tryCatch->Exception());
    
    v8::Local<v8::Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        std::cerr << fg::red << "Error: " << style::reset << *exception << std::endl;
        return;
    }
    
    // Print filename:line:column
    v8::String::Utf8Value filename(isolate_, message->GetScriptResourceName());
    int linenum = message->GetLineNumber(context_.Get(isolate_)).FromJust();
    std::cerr << fg::red << *filename << ":" << linenum << ": " << style::reset 
              << fg::yellow << *exception << style::reset << std::endl;
    
    // Print line of source code
    v8::String::Utf8Value sourceline(isolate_, 
        message->GetSourceLine(context_.Get(isolate_)).ToLocalChecked());
    std::cerr << fg::gray << *sourceline << style::reset << std::endl;
    
    // Print wavy underline
    int start = message->GetStartColumn();
    int end = message->GetEndColumn();
    std::cerr << fg::red;
    for (int i = 0; i < start; i++) {
        std::cerr << " ";
    }
    for (int i = start; i < end; i++) {
        std::cerr << "^";
    }
    std::cerr << style::reset << std::endl;
    
    // Print stack trace if available
    v8::Local<v8::Value> stack_trace_string;
    if (tryCatch->StackTrace(context_.Get(isolate_)).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString()) {
        v8::String::Utf8Value stack_trace(isolate_, stack_trace_string.As<v8::String>());
        std::cerr << fg::gray << *stack_trace << style::reset << std::endl;
    }
}