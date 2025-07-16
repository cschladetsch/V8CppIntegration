#ifndef V8_COMPAT_H
#define V8_COMPAT_H

#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>

namespace v8_compat {

// Simple wrapper - just call V8's platform creation directly
inline std::unique_ptr<v8::Platform> CreateDefaultPlatform(int thread_pool_size = 0) {
    return v8::platform::NewDefaultPlatform(thread_pool_size);
}

} // namespace v8_compat

#endif // V8_COMPAT_H