#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <v8.h>

class DllLoader {
public:
    DllLoader();
    ~DllLoader();

    // Load a DLL and expose its functions to V8
    bool LoadDll(const std::string& path, v8::Isolate* isolate, v8::Local<v8::Context> context);
    
    // Unload a specific DLL
    bool UnloadDll(const std::string& path);
    
    // Unload all DLLs
    void UnloadAll();
    
    // Hot reload - unload and reload a DLL
    bool ReloadDll(const std::string& path, v8::Isolate* isolate, v8::Local<v8::Context> context);
    
    // Get loaded DLL names
    std::vector<std::string> GetLoadedDlls() const;

private:
    struct DllHandle {
        void* handle;
        std::string path;
        std::vector<std::string> exportedFunctions;
    };
    
    std::unordered_map<std::string, std::unique_ptr<DllHandle>> loadedDlls_;
    
    // Platform-specific DLL loading
    void* LoadLibrary(const std::string& path);
    void FreeLibrary(void* handle);
    void* GetSymbol(void* handle, const std::string& name);
    
    // Register DLL functions with V8
    bool RegisterDllFunctions(void* handle, const std::string& dllName, 
                            v8::Isolate* isolate, v8::Local<v8::Context> context);
};