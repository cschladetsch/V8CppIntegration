#include "V8Compat.h"
#include <libplatform/libplatform.h>
#include <iostream>

namespace v8_compat {

// Implementation that creates V8 platform directly
std::unique_ptr<v8::Platform> CreateDefaultPlatform(int thread_pool_size) {
    // Use the standard V8 platform creation
    return v8::platform::NewDefaultPlatform(
        thread_pool_size,
        v8::platform::IdleTaskSupport::kDisabled,
        v8::platform::InProcessStackDumping::kDisabled,
        nullptr);
}

}  // namespace v8_compat