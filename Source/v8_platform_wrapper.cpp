// V8 Platform Wrapper - Bridges V8's platform API with system STL
#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>

// Define the NewDefaultPlatform function that examples expect
namespace v8 {
namespace platform {

std::unique_ptr<v8::Platform> NewDefaultPlatform(
    int thread_pool_size,
    IdleTaskSupport idle_task_support,
    InProcessStackDumping in_process_stack_dumping,
    std::unique_ptr<v8::TracingController> tracing_controller,
    PriorityMode priority_mode) {
  
  // Call V8's actual platform creation function
  return v8::platform::NewDefaultPlatform(
      thread_pool_size,
      idle_task_support, 
      in_process_stack_dumping,
      std::move(tracing_controller),
      priority_mode);
}

} // namespace platform
} // namespace v8