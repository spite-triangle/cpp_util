# 辅助工具

`util.hpp` 是一个 C++ 实用工具头文件，适用于跨平台（Windows/Linux）开发
- `defer` 资源释放
- 字符串实用功能

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

### 字符串处理

- **字符串格式化**
  - `format(fmt, ...)`：类似 `sprintf` 的格式化字符串。
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
- C++11 及以上
- Windows: `<windows.h>`
- Linux: `<iconv.h>`

## 许可协议

MIT License
