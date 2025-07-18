#ifndef V8_AUTO_REGISTER_H
#define V8_AUTO_REGISTER_H

#include <v8.h>
#include <vector>
#include <functional>
#include <string>

namespace v8reg {

using namespace v8;

class FunctionRegistry {
public:
    struct FunctionEntry {
        std::string name;
        FunctionCallback callback;
        
        FunctionEntry(const std::string& n, FunctionCallback cb) 
            : name(n), callback(cb) {}
    };
    
    static FunctionRegistry& Instance() {
        static FunctionRegistry instance;
        return instance;
    }
    
    void Register(const std::string& name, FunctionCallback callback) {
        functions_.push_back({name, callback});
    }
    
    void InstallAll(Isolate* isolate, Local<Context> context) {
        Local<Object> global = context->Global();
        
        for (const auto& entry : functions_) {
            global->Set(context,
                String::NewFromUtf8(isolate, entry.name.c_str()).ToLocalChecked(),
                Function::New(context, entry.callback).ToLocalChecked()
            ).FromJust();
        }
    }
    
    const std::vector<FunctionEntry>& GetFunctions() const {
        return functions_;
    }
    
private:
    std::vector<FunctionEntry> functions_;
};

class AutoRegister {
public:
    AutoRegister(const std::string& name, FunctionCallback callback) {
        FunctionRegistry::Instance().Register(name, callback);
    }
};

#define V8_REGISTER_FUNCTION(name, callback) \
    static v8reg::AutoRegister _v8_auto_register_##name(#name, callback)

}

#endif