#include <iostream>
#include <string>
#include <map>
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace v8;

// C++ class that will be exposed to JavaScript
class MyNativeObject {
private:
    std::map<std::string, double> data_;
    std::string name_;
    
public:
    MyNativeObject(const std::string& name) : name_(name) {}
    
    void SetValue(const std::string& key, double value) {
        data_[key] = value;
        std::cout << "[C++] " << name_ << ": Set " << key << " = " << value << std::endl;
    }
    
    double GetValue(const std::string& key) {
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return 0.0;
    }
    
    std::string GetName() const { return name_; }
};

// Wrapper functions for V8
void NativeObject_Constructor(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    if (!args.IsConstructCall()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Constructor requires 'new'").ToLocalChecked()));
        return;
    }
    
    String::Utf8Value name(isolate, args[0]);
    MyNativeObject* obj = new MyNativeObject(*name);
    
    // Store the C++ object in the JavaScript object
    args.This()->SetInternalField(0, External::New(isolate, obj));
    args.GetReturnValue().Set(args.This());
}

void NativeObject_SetValue(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    // Get the C++ object
    Local<External> wrap = Local<External>::Cast(args.This()->GetInternalField(0));
    MyNativeObject* obj = static_cast<MyNativeObject*>(wrap->Value());
    
    String::Utf8Value key(isolate, args[0]);
    double value = args[1]->NumberValue(isolate->GetCurrentContext()).ToChecked();
    
    obj->SetValue(*key, value);
}

void NativeObject_GetValue(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    Local<External> wrap = Local<External>::Cast(args.This()->GetInternalField(0));
    MyNativeObject* obj = static_cast<MyNativeObject*>(wrap->Value());
    
    String::Utf8Value key(isolate, args[0]);
    double value = obj->GetValue(*key);
    
    args.GetReturnValue().Set(Number::New(isolate, value));
}

void NativeObject_GetName(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    Local<External> wrap = Local<External>::Cast(args.This()->GetInternalField(0));
    MyNativeObject* obj = static_cast<MyNativeObject*>(wrap->Value());
    
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, 
        obj->GetName().c_str()).ToLocalChecked());
}

// Async callback example
void AsyncOperation(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    if (!args[0]->IsFunction()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Callback function required").ToLocalChecked()));
        return;
    }
    
    // Get the callback function
    Local<Function> callback = Local<Function>::Cast(args[0]);
    Global<Function> persistent_callback(isolate, callback);
    
    std::cout << "[C++] Starting async operation..." << std::endl;
    
    // Simulate async work (in real code, this would be on another thread)
    // For demonstration, we'll just call it immediately
    {
        HandleScope handle_scope(isolate);
        Local<Context> context = isolate->GetCurrentContext();
        Context::Scope context_scope(context);
        
        // Prepare callback arguments
        Local<Value> argv[] = {
            String::NewFromUtf8(isolate, "Async operation completed!").ToLocalChecked(),
            Number::New(isolate, 123.45)
        };
        
        // Call the JavaScript callback
        Local<Function> local_callback = Local<Function>::New(isolate, persistent_callback);
        local_callback->Call(context, context->Global(), 2, argv).ToLocalChecked();
    }
}

// Event emitter example
class EventEmitter {
private:
    std::map<std::string, Global<Function>> listeners_;
    Isolate* isolate_;
    
public:
    EventEmitter(Isolate* isolate) : isolate_(isolate) {}
    
    void On(const std::string& event, Local<Function> callback) {
        listeners_[event].Reset(isolate_, callback);
        std::cout << "[C++] Registered listener for event: " << event << std::endl;
    }
    
    void Emit(const std::string& event, Local<Value> data) {
        auto it = listeners_.find(event);
        if (it != listeners_.end()) {
            HandleScope handle_scope(isolate_);
            Local<Context> context = isolate_->GetCurrentContext();
            Context::Scope context_scope(context);
            
            Local<Function> callback = Local<Function>::New(isolate_, it->second);
            Local<Value> argv[] = { data };
            callback->Call(context, context->Global(), 1, argv).ToLocalChecked();
        }
    }
};

EventEmitter* g_emitter = nullptr;

void EventEmitter_On(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    String::Utf8Value event(isolate, args[0]);
    Local<Function> callback = Local<Function>::Cast(args[1]);
    
    g_emitter->On(*event, callback);
}

void EventEmitter_Emit(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    String::Utf8Value event(isolate, args[0]);
    g_emitter->Emit(*event, args[1]);
}

int main(int, char* argv[]) {
    // Initialize V8
    V8::InitializeICUDefaultLocation(argv[0]);
    V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    
    // Create isolate
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = 
        ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Create event emitter
        g_emitter = new EventEmitter(isolate);
        
        // Register native object constructor
        Local<FunctionTemplate> native_obj_template = FunctionTemplate::New(isolate, NativeObject_Constructor);
        native_obj_template->SetClassName(String::NewFromUtf8(isolate, "NativeObject").ToLocalChecked());
        native_obj_template->InstanceTemplate()->SetInternalFieldCount(1);
        
        // Add methods to prototype
        native_obj_template->PrototypeTemplate()->Set(
            String::NewFromUtf8(isolate, "setValue").ToLocalChecked(),
            FunctionTemplate::New(isolate, NativeObject_SetValue));
        
        native_obj_template->PrototypeTemplate()->Set(
            String::NewFromUtf8(isolate, "getValue").ToLocalChecked(),
            FunctionTemplate::New(isolate, NativeObject_GetValue));
        
        native_obj_template->PrototypeTemplate()->Set(
            String::NewFromUtf8(isolate, "getName").ToLocalChecked(),
            FunctionTemplate::New(isolate, NativeObject_GetName));
        
        // Register in global scope
        Local<Object> global = context->Global();
        global->Set(context,
            String::NewFromUtf8(isolate, "NativeObject").ToLocalChecked(),
            native_obj_template->GetFunction(context).ToLocalChecked()).Check();
        
        // Register async function
        global->Set(context,
            String::NewFromUtf8(isolate, "asyncOperation").ToLocalChecked(),
            Function::New(context, AsyncOperation).ToLocalChecked()).Check();
        
        // Register event emitter functions
        global->Set(context,
            String::NewFromUtf8(isolate, "on").ToLocalChecked(),
            Function::New(context, EventEmitter_On).ToLocalChecked()).Check();
        
        global->Set(context,
            String::NewFromUtf8(isolate, "emit").ToLocalChecked(),
            Function::New(context, EventEmitter_Emit).ToLocalChecked()).Check();
        
        // JavaScript code demonstrating all features
        const char* js_code = R"(
            console.log('=== Advanced V8 Example ===\n');
            
            // 1. Create and use native objects
            console.log('1. Native Object Example:');
            var obj1 = new NativeObject('DataStore1');
            var obj2 = new NativeObject('DataStore2');
            
            obj1.setValue('temperature', 23.5);
            obj1.setValue('humidity', 65.2);
            
            obj2.setValue('pressure', 1013.25);
            
            console.log('obj1 name:', obj1.getName());
            console.log('obj1 temperature:', obj1.getValue('temperature'));
            console.log('obj2 pressure:', obj2.getValue('pressure'));
            
            // 2. Async callback example
            console.log('\n2. Async Operation Example:');
            asyncOperation(function(message, value) {
                console.log('Async callback received:', message, 'with value:', value);
            });
            
            // 3. Event emitter example
            console.log('\n3. Event Emitter Example:');
            on('data', function(data) {
                console.log('Event received! Data:', data);
            });
            
            on('error', function(error) {
                console.log('Error event:', error);
            });
            
            // Trigger events from JavaScript
            console.log('Emitting events...');
            emit('data', { type: 'sensor', value: 42 });
            emit('error', 'Something went wrong!');
            
            // 4. Complex interaction
            console.log('\n4. Complex Interaction:');
            function processNativeData(nativeObj) {
                var temp = nativeObj.getValue('temperature');
                if (temp > 25) {
                    console.log('Warning: High temperature detected:', temp);
                    emit('data', { alert: 'high_temp', value: temp });
                }
                return temp * 1.8 + 32; // Convert to Fahrenheit
            }
            
            var fahrenheit = processNativeData(obj1);
            console.log('Temperature in Fahrenheit:', fahrenheit);
        )";
        
        // Execute the JavaScript code
        Local<String> source = String::NewFromUtf8(isolate, js_code).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        script->Run(context).ToLocalChecked();
        
        // Emit an event from C++
        std::cout << "\n[C++] Emitting event from C++ side..." << std::endl;
        Local<Object> data_obj = Object::New(isolate);
        data_obj->Set(context,
            String::NewFromUtf8(isolate, "source").ToLocalChecked(),
            String::NewFromUtf8(isolate, "cpp").ToLocalChecked()).Check();
        data_obj->Set(context,
            String::NewFromUtf8(isolate, "timestamp").ToLocalChecked(),
            Number::New(isolate, 1234567890)).Check();
        g_emitter->Emit("data", data_obj);
        
        delete g_emitter;
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}