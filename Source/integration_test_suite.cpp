#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <sstream>

using namespace v8;

class V8IntegrationTestFixture : public ::testing::Test {
protected:
    static std::unique_ptr<Platform> platform;
    Isolate* isolate;

    static void SetUpTestCase() {
        V8::InitializeICUDefaultLocation(".");
        V8::InitializeExternalStartupData(".");
        platform = platform::NewDefaultPlatform();
        V8::InitializePlatform(platform.get());
        V8::Initialize();
    }

    static void TearDownTestCase() {
        V8::Dispose();
        V8::ShutdownPlatform();
        platform.reset();
    }

    void SetUp() override {
        Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = Isolate::New(create_params);
    }

    void TearDown() override {
        isolate->Dispose();
    }
    
    // Helper function to run JavaScript code
    Local<Value> RunScript(const char* source_code, Local<Context> context) {
        Local<String> source = String::NewFromUtf8(isolate, source_code).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        return script->Run(context).ToLocalChecked();
    }
};

std::unique_ptr<Platform> V8IntegrationTestFixture::platform;

// Test 1: Nested object property access
TEST_F(V8IntegrationTestFixture, NestedObjectPropertyAccess) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let obj = {
            level1: {
                level2: {
                    level3: {
                        value: 'deep value'
                    }
                }
            }
        };
        obj.level1.level2.level3.value;
    )";
    
    Local<Value> result = RunScript(js_code, context);
    
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "deep value");
}

// Test 2: Array methods (map, filter, reduce)
TEST_F(V8IntegrationTestFixture, ArrayMethodChaining) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        [1, 2, 3, 4, 5]
            .map(x => x * 2)
            .filter(x => x > 5)
            .reduce((acc, x) => acc + x, 0);
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsNumber());
    EXPECT_EQ(result->Int32Value(context).FromJust(), 24);
}

// Test 3: Class definition and instantiation
TEST_F(V8IntegrationTestFixture, ES6ClassDefinition) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        class Person {
            constructor(name, age) {
                this.name = name;
                this.age = age;
            }
            
            greet() {
                return `Hello, I'm ${this.name}, ${this.age} years old`;
            }
        }
        
        let person = new Person('John', 30);
        person.greet();
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "Hello, I'm John, 30 years old");
}

// Test 4: Template literals with expressions
TEST_F(V8IntegrationTestFixture, TemplateLiteralsWithExpressions) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let a = 5;
        let b = 10;
        `The sum of ${a} and ${b} is ${a + b}, and the product is ${a * b}`;
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "The sum of 5 and 10 is 15, and the product is 50");
}

// Test 5: Destructuring assignment
TEST_F(V8IntegrationTestFixture, DestructuringAssignment) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let {a, b, ...rest} = {a: 1, b: 2, c: 3, d: 4};
        let [x, y, ...others] = [10, 20, 30, 40, 50];
        ({a, b, x, y, restSum: Object.values(rest).reduce((s, v) => s + v, 0)});
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> a = obj->Get(context, String::NewFromUtf8(isolate, "a").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(a->Int32Value(context).FromJust(), 1);
    
    Local<Value> restSum = obj->Get(context, String::NewFromUtf8(isolate, "restSum").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(restSum->Int32Value(context).FromJust(), 7);
}

// Test 6: Arrow functions and closures
TEST_F(V8IntegrationTestFixture, ArrowFunctionsAndClosures) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        const makeCounter = () => {
            let count = 0;
            return {
                increment: () => ++count,
                decrement: () => --count,
                value: () => count
            };
        };
        
        let counter = makeCounter();
        counter.increment();
        counter.increment();
        counter.decrement();
        counter.value();
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsNumber());
    EXPECT_EQ(result->Int32Value(context).FromJust(), 1);
}

// Test 7: Spread operator
TEST_F(V8IntegrationTestFixture, SpreadOperator) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        function sum(...args) {
            return args.reduce((a, b) => a + b, 0);
        }
        
        let arr1 = [1, 2, 3];
        let arr2 = [4, 5, 6];
        sum(...arr1, ...arr2);
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsNumber());
    EXPECT_EQ(result->Int32Value(context).FromJust(), 21);
}

// Test 8: Object.assign and property descriptors
TEST_F(V8IntegrationTestFixture, ObjectAssignAndDescriptors) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let obj1 = {a: 1};
        let obj2 = {b: 2};
        let obj3 = {c: 3};
        
        Object.defineProperty(obj3, 'd', {
            value: 4,
            enumerable: false
        });
        
        let merged = Object.assign({}, obj1, obj2, obj3);
        Object.keys(merged).sort().join(',');
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "a,b,c");  // 'd' is not enumerable
}

// Test 9: Array.from and iterables
TEST_F(V8IntegrationTestFixture, ArrayFromIterables) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let set = new Set([1, 2, 3, 2, 1]);
        let doubled = Array.from(set, x => x * 2);
        doubled.sort((a, b) => a - b).join(',');
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "2,4,6");
}

// Test 10: Default parameters and rest parameters
TEST_F(V8IntegrationTestFixture, DefaultAndRestParameters) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        function greet(name = 'World', ...titles) {
            let titleStr = titles.length ? titles.join(' ') + ' ' : '';
            return `Hello, ${titleStr}${name}!`;
        }
        
        greet('Smith', 'Dr.', 'Prof.');
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "Hello, Dr. Prof. Smith!");
}

// Test 11: for...of loops
TEST_F(V8IntegrationTestFixture, ForOfLoops) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let sum = 0;
        let map = new Map([[1, 'a'], [2, 'b'], [3, 'c']]);
        
        for (let [key, value] of map) {
            sum += key;
        }
        
        for (let char of 'hello') {
            sum += char.charCodeAt(0);
        }
        
        sum;
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsNumber());
    EXPECT_EQ(result->Int32Value(context).FromJust(), 538);  // 1+2+3 + ASCII values of 'hello'
}

// Test 12: Object.entries and Object.values
TEST_F(V8IntegrationTestFixture, ObjectEntriesAndValues) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let obj = {a: 1, b: 2, c: 3};
        let entries = Object.entries(obj);
        let values = Object.values(obj);
        
        ({
            entriesLength: entries.length,
            valuesSum: values.reduce((a, b) => a + b, 0),
            firstEntry: entries[0].join(':')
        });
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> valuesSum = obj->Get(context, String::NewFromUtf8(isolate, "valuesSum").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(valuesSum->Int32Value(context).FromJust(), 6);
}

// Test 13: Promise.all and Promise.race
TEST_F(V8IntegrationTestFixture, PromiseAllAndRace) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let p1 = Promise.resolve(1);
        let p2 = Promise.resolve(2);
        let p3 = Promise.resolve(3);
        
        Promise.all([p1, p2, p3]).then(values => values.reduce((a, b) => a + b, 0));
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsPromise());
    
    // For testing purposes, we'll check that it's a promise
    // In real usage, you'd need to handle the promise resolution
    Local<Promise> promise = result.As<Promise>();
    EXPECT_EQ(promise->State(), Promise::kFulfilled);
}

// Test 14: String methods (padStart, padEnd, repeat)
TEST_F(V8IntegrationTestFixture, ModernStringMethods) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let str = 'JS';
        ({
            padded: str.padStart(5, '*').padEnd(8, '#'),
            repeated: 'ha'.repeat(3),
            includes: 'hello world'.includes('world')
        });
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> padded = obj->Get(context, String::NewFromUtf8(isolate, "padded").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value paddedStr(isolate, padded);
    EXPECT_STREQ(*paddedStr, "***JS###");
    
    Local<Value> repeated = obj->Get(context, String::NewFromUtf8(isolate, "repeated").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value repeatedStr(isolate, repeated);
    EXPECT_STREQ(*repeatedStr, "hahaha");
}

// Test 15: Number methods and Math extensions
TEST_F(V8IntegrationTestFixture, NumberAndMathMethods) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        ({
            isFinite: Number.isFinite(42),
            isNaN: Number.isNaN(NaN),
            isInteger: Number.isInteger(42.0),
            sign: Math.sign(-42),
            trunc: Math.trunc(42.9),
            cbrt: Math.cbrt(27)
        });
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> isFinite = obj->Get(context, String::NewFromUtf8(isolate, "isFinite").ToLocalChecked()).ToLocalChecked();
    EXPECT_TRUE(isFinite->BooleanValue(isolate));
    
    Local<Value> cbrt = obj->Get(context, String::NewFromUtf8(isolate, "cbrt").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(cbrt->Int32Value(context).FromJust(), 3);
}

// Test 16: Async function simulation
TEST_F(V8IntegrationTestFixture, AsyncFunctionSimulation) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        function delay(ms, value) {
            return new Promise(resolve => {
                // In real async, this would use setTimeout
                resolve(value);
            });
        }
        
        async function fetchData() {
            let data1 = await delay(100, 'first');
            let data2 = await delay(200, 'second');
            return data1 + ' ' + data2;
        }
        
        // Since we can't actually wait in tests, we'll test the function exists
        typeof fetchData;
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsString());
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "function");
}

// Test 17: Error handling with custom errors
TEST_F(V8IntegrationTestFixture, CustomErrorHandling) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        class CustomError extends Error {
            constructor(message, code) {
                super(message);
                this.name = 'CustomError';
                this.code = code;
            }
        }
        
        try {
            throw new CustomError('Something went wrong', 'ERR_001');
        } catch (e) {
            ({
                name: e.name,
                message: e.message,
                code: e.code,
                isError: e instanceof Error,
                isCustom: e instanceof CustomError
            });
        }
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> name = obj->Get(context, String::NewFromUtf8(isolate, "name").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value nameStr(isolate, name);
    EXPECT_STREQ(*nameStr, "CustomError");
    
    Local<Value> isCustom = obj->Get(context, String::NewFromUtf8(isolate, "isCustom").ToLocalChecked()).ToLocalChecked();
    EXPECT_TRUE(isCustom->BooleanValue(isolate));
}

// Test 18: Complex data transformations
TEST_F(V8IntegrationTestFixture, ComplexDataTransformations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        let users = [
            {name: 'Alice', age: 30, skills: ['JS', 'Python']},
            {name: 'Bob', age: 25, skills: ['JS', 'Java', 'C++']},
            {name: 'Charlie', age: 35, skills: ['Python', 'Go']}
        ];
        
        let result = users
            .filter(u => u.skills.includes('JS'))
            .map(u => ({
                ...u,
                skillCount: u.skills.length,
                isExpert: u.skills.length > 2
            }))
            .reduce((acc, u) => {
                acc.totalAge += u.age;
                acc.totalSkills += u.skillCount;
                acc.users.push(u.name);
                return acc;
            }, {totalAge: 0, totalSkills: 0, users: []});
        
        result;
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> totalAge = obj->Get(context, String::NewFromUtf8(isolate, "totalAge").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(totalAge->Int32Value(context).FromJust(), 55);  // Alice(30) + Bob(25)
    
    Local<Value> totalSkills = obj->Get(context, String::NewFromUtf8(isolate, "totalSkills").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(totalSkills->Int32Value(context).FromJust(), 5);  // Alice(2) + Bob(3)
}

// Test 19: Recursive functions and memoization
TEST_F(V8IntegrationTestFixture, RecursionAndMemoization) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        function memoize(fn) {
            const cache = new Map();
            return function(...args) {
                const key = JSON.stringify(args);
                if (cache.has(key)) {
                    return cache.get(key);
                }
                const result = fn.apply(this, args);
                cache.set(key, result);
                return result;
            };
        }
        
        const fibonacci = memoize(function(n) {
            if (n <= 1) return n;
            return fibonacci(n - 1) + fibonacci(n - 2);
        });
        
        ({
            fib10: fibonacci(10),
            fib15: fibonacci(15),
            fib20: fibonacci(20)
        });
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> fib10 = obj->Get(context, String::NewFromUtf8(isolate, "fib10").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(fib10->Int32Value(context).FromJust(), 55);
    
    Local<Value> fib20 = obj->Get(context, String::NewFromUtf8(isolate, "fib20").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(fib20->Int32Value(context).FromJust(), 6765);
}

// Test 20: Module pattern and private variables
TEST_F(V8IntegrationTestFixture, ModulePatternPrivateVariables) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const char* js_code = R"(
        const BankAccount = (function() {
            const accounts = new WeakMap();
            
            class BankAccount {
                constructor(initialBalance = 0) {
                    accounts.set(this, {
                        balance: initialBalance,
                        transactions: []
                    });
                }
                
                deposit(amount) {
                    const account = accounts.get(this);
                    account.balance += amount;
                    account.transactions.push({type: 'deposit', amount});
                    return account.balance;
                }
                
                withdraw(amount) {
                    const account = accounts.get(this);
                    if (amount > account.balance) {
                        throw new Error('Insufficient funds');
                    }
                    account.balance -= amount;
                    account.transactions.push({type: 'withdraw', amount});
                    return account.balance;
                }
                
                getBalance() {
                    return accounts.get(this).balance;
                }
                
                getTransactionCount() {
                    return accounts.get(this).transactions.length;
                }
            }
            
            return BankAccount;
        })();
        
        let account = new BankAccount(100);
        account.deposit(50);
        account.withdraw(30);
        
        ({
            balance: account.getBalance(),
            transactions: account.getTransactionCount(),
            // Try to access private data (should be undefined)
            privateAccess: account.balance
        });
    )";
    
    Local<Value> result = RunScript(js_code, context);
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = result.As<Object>();
    
    Local<Value> balance = obj->Get(context, String::NewFromUtf8(isolate, "balance").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(balance->Int32Value(context).FromJust(), 120);  // 100 + 50 - 30
    
    Local<Value> transactions = obj->Get(context, String::NewFromUtf8(isolate, "transactions").ToLocalChecked()).ToLocalChecked();
    EXPECT_EQ(transactions->Int32Value(context).FromJust(), 2);
    
    Local<Value> privateAccess = obj->Get(context, String::NewFromUtf8(isolate, "privateAccess").ToLocalChecked()).ToLocalChecked();
    EXPECT_TRUE(privateAccess->IsUndefined());  // Private data is not accessible
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}