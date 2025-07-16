#include "DllLoader.h"
#include <iostream>
#include <algorithm>
#include <rang/rang.hpp>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

DllLoader::DllLoader() {}

DllLoader::~DllLoader() {
    UnloadAll();
}

bool DllLoader::LoadDll(const std::string& path, v8::Isolate* isolate, v8::Local<v8::Context> context) {
    // Check if already loaded
    if (loadedDlls_.find(path) != loadedDlls_.end()) {
        std::cerr << rang::fg::yellow << "DLL already loaded: " << path << rang::style::reset << std::endl;
        return false;
    }
    
    // Load the library
    void* handle = LoadLibrary(path);
    if (!handle) {
        std::cerr << rang::fg::red << "Failed to load DLL: " << path << rang::style::reset << std::endl;
        return false;
    }
    
    // Create DLL handle
    auto dllHandle = std::make_unique<DllHandle>();
    dllHandle->handle = handle;
    dllHandle->path = path;
    
    // Register functions with V8
    if (!RegisterDllFunctions(handle, path, isolate, context)) {
        FreeLibrary(handle);
        return false;
    }
    
    // Store the handle
    loadedDlls_[path] = std::move(dllHandle);
    std::cout << rang::fg::green << "Successfully loaded DLL: " << path << rang::style::reset << std::endl;
    return true;
}

bool DllLoader::UnloadDll(const std::string& path) {
    auto it = loadedDlls_.find(path);
    if (it == loadedDlls_.end()) {
        return false;
    }
    
    FreeLibrary(it->second->handle);
    loadedDlls_.erase(it);
    std::cout << "Unloaded DLL: " << path << std::endl;
    return true;
}

void DllLoader::UnloadAll() {
    for (auto& pair : loadedDlls_) {
        FreeLibrary(pair.second->handle);
    }
    loadedDlls_.clear();
}

bool DllLoader::ReloadDll(const std::string& path, v8::Isolate* isolate, v8::Local<v8::Context> context) {
    UnloadDll(path);
    return LoadDll(path, isolate, context);
}

std::vector<std::string> DllLoader::GetLoadedDlls() const {
    std::vector<std::string> result;
    for (const auto& pair : loadedDlls_) {
        result.push_back(pair.first);
    }
    return result;
}

void* DllLoader::LoadLibrary(const std::string& path) {
#ifdef _WIN32
    return ::LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void DllLoader::FreeLibrary(void* handle) {
    if (!handle) return;
    
#ifdef _WIN32
    ::FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

void* DllLoader::GetSymbol(void* handle, const std::string& name) {
    if (!handle) return nullptr;
    
#ifdef _WIN32
    return ::GetProcAddress(static_cast<HMODULE>(handle), name.c_str());
#else
    return dlsym(handle, name.c_str());
#endif
}

bool DllLoader::RegisterDllFunctions(void* handle, const std::string& dllName, 
                                   v8::Isolate* isolate, v8::Local<v8::Context> context) {
    // Look for the exported V8 registration function
    // Convention: DLLs should export "RegisterV8Functions" function
    typedef void (*RegisterFunc)(v8::Isolate*, v8::Local<v8::Context>);
    
    RegisterFunc registerFunc = reinterpret_cast<RegisterFunc>(GetSymbol(handle, "RegisterV8Functions"));
    if (!registerFunc) {
        std::cerr << rang::fg::red << "DLL does not export RegisterV8Functions: " << dllName << rang::style::reset << std::endl;
        return false;
    }
    
    // Call the registration function
    try {
        registerFunc(isolate, context);
        return true;
    } catch (...) {
        std::cerr << "Exception thrown while registering functions from: " << dllName << std::endl;
        return false;
    }
}