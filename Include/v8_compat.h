#ifndef V8_COMPAT_H
#define V8_COMPAT_H

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>

namespace v8_compat {

// Wrapper function that creates a platform compatible with different V8 versions
inline std::unique_ptr<v8::Platform> CreateDefaultPlatform(
    int thread_pool_size = 0) {
    
    // Use the basic platform creation that works with system V8
    return v8::platform::NewDefaultPlatform(thread_pool_size);
}

// Initialize V8 with proper setup
inline bool InitializeV8() {
    v8::V8::InitializeICU();
    return true;
}

} // namespace v8_compat

#endif // V8_COMPAT_H