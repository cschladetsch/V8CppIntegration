#ifndef V8_COMPAT_H
#define V8_COMPAT_H

#include <v8.h>
#include <v8-platform.h>
#include <memory>

namespace v8_compat {

// Wrapper function that creates a platform with matching ABI
inline std::unique_ptr<v8::Platform> CreateDefaultPlatform(
    int thread_pool_size = 0,
    v8::platform::IdleTaskSupport idle_task_support = v8::platform::IdleTaskSupport::kDisabled,
    v8::platform::InProcessStackDumping in_process_stack_dumping = v8::platform::InProcessStackDumping::kDisabled) {
    
    // Since we can't directly call NewDefaultPlatform due to ABI mismatch,
    // we'll need to use a different approach
    
    // For now, return nullptr and handle platform creation differently
    return nullptr;
}

// Alternative initialization that doesn't require platform
inline bool InitializeV8() {
    // V8 requires a platform, but we can try minimal initialization
    // This is a workaround for the ABI issue
    v8::V8::InitializeICU();
    return true;
}

} // namespace v8_compat

#endif // V8_COMPAT_H