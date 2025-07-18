#ifndef V8_MODULE_SYSTEM_H
#define V8_MODULE_SYSTEM_H

#include <v8.h>
#include <string>
#include <vector>
#include <map>

namespace v8mod {

using namespace v8;

struct FunctionInfo {
    std::string name;
    FunctionCallback callback;
    std::string description;
};

struct ModuleInfo {
    std::string name;
    std::string version;
    std::vector<FunctionInfo> functions;
};

class V8Module {
public:
    V8Module(const std::string& name, const std::string& version = "1.0.0") 
        : info_{name, version, {}} {}
    
    V8Module& AddFunction(const std::string& name, 
                         FunctionCallback callback, 
                         const std::string& description = "") {
        info_.functions.push_back({name, callback, description});
        return *this;
    }
    
    void Install(Isolate* isolate, Local<Context> context) {
        Local<Object> global = context->Global();
        Local<Object> module = Object::New(isolate);
        
        for (const auto& func : info_.functions) {
            module->Set(context,
                String::NewFromUtf8(isolate, func.name.c_str()).ToLocalChecked(),
                Function::New(context, func.callback).ToLocalChecked()
            ).FromJust();
        }
        
        global->Set(context,
            String::NewFromUtf8(isolate, info_.name.c_str()).ToLocalChecked(),
            module
        ).FromJust();
    }
    
    const ModuleInfo& GetInfo() const { return info_; }
    
private:
    ModuleInfo info_;
};

template<typename T>
class ModuleRegistry {
public:
    static V8Module& GetModule() {
        static V8Module module(T::GetModuleName(), T::GetModuleVersion());
        static bool initialized = false;
        if (!initialized) {
            T::RegisterFunctions(module);
            initialized = true;
        }
        return module;
    }
};

#define DEFINE_V8_MODULE(className, moduleName, moduleVersion) \
    class className { \
    public: \
        static const char* GetModuleName() { return moduleName; } \
        static const char* GetModuleVersion() { return moduleVersion; } \
        static void RegisterFunctions(v8mod::V8Module& module); \
    }; \
    static auto& _module_##className = v8mod::ModuleRegistry<className>::GetModule()

}

#endif