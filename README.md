# 辅助工具

`util.hpp` 是一个 C++ 实用工具头文件，适用于跨平台（Windows/Linux）开发
- `defer` 资源释放
- 字符串实用功能
- 提供 `python like` 的内置异常
- 常用线程安全的数据结构
- 简易的 `optional` 实现，可与 `std::optional` 互换

## 功能介绍

### defer

类似 `go` 中的 `defer` 关键字，利用 `RAII` 可实现资源全路径释放

```cpp
TEST_CASE("defer"){
    printf("begin\n");

    RAII_DEFER(
        printf("defer\n");
    );

    printf("end\n");

    /*
    打印顺序:
      begin
      end
      defer
    */
}
```
### 通用工具

- `util::Singleton`: 快速单列

### 字符串处理

- **字符串格式化**
  - `format(fmt, ...)`：类似 `sprintf` 的格式化字符串。
  - `joint`: 字符串拼接，支持 `iomanip` 
  - `join`: 拼接数据
- **编码转换**
  - `fromUtf8` / `toUtf8`：UTF-8 与宽字符串（`std::wstring`）互转。
  - `fromAnsi` / `toAnsi`：ANSI 与宽字符串互转。
  - `ansi2utf8` / `utf82ansi`：ANSI 与 UTF-8 互转。
- **字符串大小写**
  - `toLower` / `toUpper`：字符串转小写/大写。
- **字符串查找**
  - `startWith` / `endWith`：判断字符串前缀/后缀（支持字符串和正则，支持大小写敏感）。
  - `contain`：判断字符串是否包含子串或正则。
- **字符串替换**
  - `repleace`：字符串替换（支持字符串和正则，支持替换全部或第一个）。
- **字符串分割**
  - `split`：按分隔符或正则分割字符串。
- **字符串修剪**
  - `ltrim` / `rtrim` / `trims`：去除字符串左/右/两端空白。
- **字符串转数值**
  - `toNumber`：将字符串安全转换为各种数值类型。
- **可视化**
  - `dumpMemeory`: 将二进制内存转换为字符串，便于调试

### 异常

仿照 `python` 结构定义异常，并基于 [https://github.com/bombela/backward-cpp](https://github.com/bombela/backward-cpp) 实现堆栈信息追踪

```txt
BaseException
 └── Exception
      ├── ArithmeticError
      │    ├── FloatingPointError
      │    ├── OverflowError
      │    └── ZeroDivisionError
      ├── AssertionError
      ├── AttributeError
      ├── BufferError
      ├── EOFError
      ├── ImportError
      │    └── ModuleNotFoundError
      ├── LookupError
      │    ├── IndexError
      │    └── KeyError
      ├── MemoryError
      ├── NameError
      │    └── UnboundLocalError
      ├── OSError
      │    ├── BlockingIOError
      │    ├── ChildProcessError
      │    ├── ConnectionError
      │    │    ├── BrokenPipeError
      │    │    ├── ConnectionAbortedError
      │    │    ├── ConnectionRefusedError
      │    │    └── ConnectionResetError
      │    ├── FileExistsError
      │    ├── FileNotFoundError
      │    ├── InterruptedError
      │    ├── IsADirectoryError
      │    ├── NotADirectoryError
      │    ├── PermissionError
      │    ├── ProcessLookupError
      │    └── TimeoutError
      ├── ReferenceError
      ├── RuntimeError
      │    ├── NotImplementedError
      │    ├── PythonFinalizationError
      │    └── RecursionError
      ├── StopAsyncIteration
      ├── StopIteration
      ├── SyntaxError
      │    └── IndentationError
      │         └── TabError
      ├── SystemError
      ├── TypeError
      └── ValueError
        └── UnicodeError
            ├── UnicodeDecodeError
            ├── UnicodeEncodeError
            └── UnicodeTranslateError
```

捕获到当异常时，可打印堆栈信息

```cpp
TEST_CASE("stack_exception"){

    try{
        fcn1(0,2);
    }catch(const util::ValueError & e){
        printf("%s", e.detail().c_str());
    }
}
```

```term
triangle@LEARN:~$ ./demo
error: xxx
Stack trace (most recent call last):
#4    Object "", at 0x3, in  ?? 
#3    Object "", at 0x100000011, in  ?? 
#2    Object "", at 0xddf09ff230, in  ?? 
#1    Object "", at 0x25800000000, in  ?? 
#0    Object "", at 0x7ff724fc03c5, in  ?? 
```

> [!note]
> 堆栈信息使用 [backward-cpp]( https://github.com/bombela/backward-cpp) 库捕获
> - `window` 需要依赖 `dbghelp` 库
> - `linux` 需要依赖 `libunwind` 库

### optional

```cpp
TEST_CASE("optional"){
    util::optional<int> opt1;
    CHECK(opt1.has_value() == false);
    CHECK(opt1 == util::nullopt);

    opt1.emplace(1);
    CHECK(opt1.has_value() == true);

    util::optional<int> opt2(1);
    CHECK(opt1 == opt2);
}
```

### 数据结构

- `util::SafeLinkQueue` : 线程安全队列

### Cancelation

仿照 `vscode` 中组件实现，可实现复杂任务流程的取消

```cpp
TEST_CASE("cancellation_parallel"){
    // 取消 token 管理器
    auto tokenSource = std::make_shared<util::CancellationTokenSource>();

    // 线程任务 1
    auto t1 = std::thread([tokenSource](){
        auto token = tokenSource->token();

        token->onCancellationRequested([&](){
            std::cout << "Cancellation requested in thread 1" << std::endl;
        });

        while (!token->isCancellationRequested()) {
            std::cout << "Thread 1 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // 线程任务 2
    auto t2 = std::thread([tokenSource](){
        auto token = tokenSource->token();
        while (!token->isCancellationRequested()) {
            std::cout << "Thread 2 is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    tokenSource->token()->onCancellationRequested([&](){
        std::cout << "Cancellation requested in main thread" << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 取消任务
    tokenSource->cancel();

    t1.join();
    t2.join();
}
```

## 平台支持
- Windows：使用 Windows API 进行编码转换。
- Linux：使用 iconv 进行编码转换。

## 使用示例

```cpp
#include "util.hpp"
#include <iostream>

int main() {
    std::string s = "  Hello, World!  ";
    std::cout << util::trims(s) << std::endl; // "Hello, World!"

    std::string lower = util::toLower("ABC"); // "abc"
    std::string replaced = util::repleace("foo bar foo", "foo", "baz", true); // "baz bar baz"

    int num;
    if (util::toNumber("123", num)) {
        std::cout << num << std::endl;
    }
    return 0;
}
```

## 依赖
- `c++11` 及以上
- Windows: `<windows.h>`
- Linux: `<iconv.h>`

## 许可协议

MIT License
