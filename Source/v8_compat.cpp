#include "v8_compat.h"

namespace v8_compat {

// Implementation that calls V8's platform creation
std::unique_ptr<v8::Platform> CreateDefaultPlatform(int thread_pool_size) {
    // Call with minimal parameters to avoid ABI issues
    auto platform = v8::platform::NewDefaultPlatform(
        thread_pool_size,
        v8::platform::IdleTaskSupport::kDisabled,
        v8::platform::InProcessStackDumping::kDisabled
    );
    return std::unique_ptr<v8::Platform>(platform.release());
}

} // namespace v8_compat