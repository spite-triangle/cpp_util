#ifndef UTILS_HPP
#define UTILS_HPP

#include <regex>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <algorithm>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _linux_
#include <iconv.h>
#endif

/* ======================== micro ============================= */

// 字符串处理宏
#define CONCAT_INTERNAL(x, y) x##y
#define CONCAT(x, y) CONCAT_INTERNAL(x, y)
#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

// defer 功能实现
namespace util{
struct _DeferBase{
    virtual ~_DeferBase() = default;
};

template<typename Defer>
struct _DeferImpl: public _DeferBase{
    _DeferImpl(Defer && defer) :_defer(std::move(defer)){}
    virtual ~_DeferImpl(){ _defer(); }
    Defer _defer;
};

struct RaiiDefer{
    template<class Defer>
    RaiiDefer(Defer && defer) : _base{ new _DeferImpl<Defer>(std::move(defer))} {}
    ~RaiiDefer(){ if (_base != nullptr) delete _base; }
    _DeferBase* _base = nullptr;
};
}
#define RAII_DEFER(_defer) util::RaiiDefer CONCAT(_raii_defer_, __LINE__)([&](){_defer})

/* ====================== exception =========================== */

namespace util
{

// Exception hierarchy (Python-like exceptions mapped to C++)
// Skipped: BaseExceptionGroup, GeneratorExit, KeyboardInterrupt, SystemExit (Python-specific)
// Aliased from stdexcept: logic_error, runtime_error, etc.

using BaseException = std::exception;

class Exception : public std::exception{

public:
    Exception() : std::exception(), m_message("") {}
    Exception(const std::string & msg): std::exception(), m_message(msg) {}
    virtual ~Exception() = default;
    
    virtual const char* what() const noexcept {
        return m_message.c_str();
    }

protected:
    std::string m_message;
};


// ArithmeticError - implemented as BaseException (no std::arithmetic_error exists)
class ArithmeticError : public Exception {
public:
    ArithmeticError(const std::string& msg) : Exception(msg) {}
};

class FloatingPointError : public ArithmeticError {
public:
    FloatingPointError(const std::string& msg) : ArithmeticError(msg) {}
};

class OverflowError : public ArithmeticError {
public:
    OverflowError(const std::string& msg) : ArithmeticError(msg){}
};

class ZeroDivisionError : public ArithmeticError {
public:
    ZeroDivisionError(const std::string& msg) : ArithmeticError(msg) {}
};

class AssertionError : public Exception {
public:
    AssertionError(const std::string& msg) : Exception(msg) {}
};

class AttributeError : public Exception {
public:
    AttributeError(const std::string& msg) : Exception(msg) {}
};

class BufferError : public Exception {
public:
    BufferError(const std::string& msg) : Exception(msg) {}
};

class EOFError : public Exception {
public:
    EOFError(const std::string& msg) : Exception(msg) {}
};

// ImportError
class ImportError : public Exception {
public:
    ImportError(const std::string& msg) : Exception(msg) {}
};

class ModuleNotFoundError : public ImportError {
public:
    ModuleNotFoundError(const std::string& msg) : ImportError(msg) {}
};

// LookupError
class LookupError : public Exception {
public:
    LookupError(const std::string& msg) : Exception(msg) {}
};

class IndexError : public LookupError {
public:
    IndexError(const std::string& msg) : LookupError(msg) {}
};

class KeyError : public LookupError {
public:
    KeyError(const std::string& msg) : LookupError(msg) {}
};

class MemoryError : public Exception {
public:
    MemoryError(const std::string& msg) : Exception(msg) {}
};

// NameError
class NameError : public Exception {
public:
    NameError(const std::string& msg) : Exception(msg) {}
};

class UnboundLocalError : public NameError {
public:
    UnboundLocalError(const std::string& msg) : NameError(msg) {}
};

// OSError - custom implementation (std::system_error requires error code)
class OSError : public Exception {
public:
    OSError(const std::string& msg) : Exception(msg) {}
};

class BlockingIOError : public OSError {
public:
    BlockingIOError(const std::string& msg) : OSError(msg) {}
};

class ChildProcessError : public OSError {
public:
    ChildProcessError(const std::string& msg) : OSError(msg) {}
};

// ConnectionError
class ConnectionError : public OSError {
public:
    ConnectionError(const std::string& msg) : OSError(msg) {}
};

class BrokenPipeError : public ConnectionError {
public:
    BrokenPipeError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionAbortedError : public ConnectionError {
public:
    ConnectionAbortedError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionRefusedError : public ConnectionError {
public:
    ConnectionRefusedError(const std::string& msg) : ConnectionError(msg) {}
};

class ConnectionResetError : public ConnectionError {
public:
    ConnectionResetError(const std::string& msg) : ConnectionError(msg) {}
};

class FileExistsError : public OSError {
public:
    FileExistsError(const std::string& msg) : OSError(msg) {}
};

class FileNotFoundError : public OSError {
public:
    FileNotFoundError(const std::string& msg) : OSError(msg) {}
};

class InterruptedError : public OSError {
public:
    InterruptedError(const std::string& msg) : OSError(msg) {}
};

class IsADirectoryError : public OSError {
public:
    IsADirectoryError(const std::string& msg) : OSError(msg) {}
};

class NotADirectoryError : public OSError {
public:
    NotADirectoryError(const std::string& msg) : OSError(msg) {}
};

class PermissionError : public OSError {
public:
    PermissionError(const std::string& msg) : OSError(msg) {}
};

class ProcessLookupError : public OSError {
public:
    ProcessLookupError(const std::string& msg) : OSError(msg) {}
};

class TimeoutError : public OSError {
public:
    TimeoutError(const std::string& msg) : OSError(msg) {}
};

class ReferenceError : public Exception {
public:
    ReferenceError(const std::string& msg) : Exception(msg) {}
};

// RuntimeError
class RuntimeError : public Exception{
public:
    RuntimeError(const std::string& msg) : Exception(msg){}
};

class NotImplementedError : public RuntimeError {
public:
    NotImplementedError(const std::string& msg) : RuntimeError(msg) {}
};

class RecursionError : public RuntimeError {
public:
    RecursionError(const std::string& msg) : RuntimeError(msg) {}
};

class StopIteration : public Exception {
public:
    StopIteration(const std::string& msg) : Exception(msg) {}
};

// SyntaxError
class SyntaxError : public Exception {
public:
    SyntaxError(const std::string& msg) : Exception(msg) {}
};

class IndentationError : public SyntaxError {
public:
    IndentationError(const std::string& msg) : SyntaxError(msg) {}
};

class TabError : public IndentationError {
public:
    TabError(const std::string& msg) : IndentationError(msg) {}
};

class SystemError : public Exception {
public:
    SystemError(const std::string& msg) : Exception(msg) {}
};

class TypeError : public Exception {
public:
    TypeError(const std::string& msg) : Exception(msg) {}
};

// ValueError
class ValueError : public Exception {
public:
    ValueError(const std::string& msg) : Exception(msg) {}
};

class UnicodeError : public ValueError {
public:
    UnicodeError(const std::string& msg) : ValueError(msg) {}
};

class UnicodeDecodeError : public UnicodeError {
public:
    UnicodeDecodeError(const std::string& msg) : UnicodeError(msg) {}
};

class UnicodeEncodeError : public UnicodeError {
public:
    UnicodeEncodeError(const std::string& msg) : UnicodeError(msg) {}
};

class UnicodeTranslateError : public UnicodeError {
public:
    UnicodeTranslateError(const std::string& msg) : UnicodeError(msg) {}
};

// Warning hierarchy (skipped as Python-specific)

} // namespace util

/* ========================== common ======================== */

namespace util
{

template<class Type>
class Singleton
{
public:
    using Ptr = std::shared_ptr<Type>; 
public:
    static Ptr instance(){
        if(m_instance == nullptr){
            std::lock_guard<std::mutex> lock(m_mut);
            if(m_instance == nullptr){
                m_instance = std::make_shared<Type>();
            }
        }
        return m_instance;
    }

    static void release(){
        m_instance = nullptr;
    }

private:

    static Ptr m_instance;
    static std::mutex m_mut;
};

template<class Type>
typename Singleton<Type>::Ptr Singleton<Type>::m_instance = nullptr;

template<class Type>
std::mutex Singleton<Type>::m_mut;

} // namespace util



/* ======================== optional ========================= */

#if __cplusplus >= 201703L
#include <optional>
#endif

namespace util
{

// std::optional
#if __cplusplus >= 201703L
template <typename T>
using optional = std::optional<T>;

using bad_optional_access = std::bad_optional_access;

constexpr std::nullopt_t nullopt = std::nullopt;

template <typename T>
constexpr auto make_optional(T &&value)
{
    return std::make_optional(std::forward<T>(value));
}
#else

// nullopt type and constant
struct nullopt_t
{
    explicit constexpr nullopt_t(int) noexcept {}
};

constexpr nullopt_t nullopt{0};

class bad_optional_access : public util::RuntimeError
{
public:
    bad_optional_access() : util::RuntimeError("bad optional access") {}
};

template <typename T>
class optional
{
private:
    bool has_value_;
    alignas(T) unsigned char storage_[sizeof(T)];

    // 获取存储对象的引用
    T &value_ref()
    {
        return *reinterpret_cast<T *>(storage_);
    }

    const T &value_ref() const
    {
        return *reinterpret_cast<const T *>(storage_);
    }

public:
    // 默认构造函数 - 创建空的 optional
    optional() noexcept : has_value_(false) {}

    // 构造函数 - 从 nullopt 构造
    optional(nullopt_t) noexcept : has_value_(false) {}

    // 构造函数 - 从值构造
    optional(const T &value) : has_value_(true)
    {
        new (storage_) T(value);
    }

    optional(T &&value) : has_value_(true)
    {
        new (storage_) T(std::move(value));
    }

    // 拷贝构造函数
    optional(const optional &other) : has_value_(other.has_value_)
    {
        if (has_value_)
        {
            new (storage_) T(other.value_ref());
        }
    }

    // 移动构造函数
    optional(optional &&other) noexcept : has_value_(other.has_value_)
    {
        if (has_value_)
        {
            new (storage_) T(std::move(other.value_ref()));
        }
    }

    // 析构函数
    ~optional()
    {
        if (has_value_)
        {
            value_ref().~T();
        }
    }

    // 拷贝赋值运算符
    optional &operator=(const optional &other)
    {
        if (this != &other)
        {
            if (has_value_)
            {
                value_ref().~T();
            }
            has_value_ = other.has_value_;
            if (has_value_)
            {
                new (storage_) T(other.value_ref());
            }
        }
        return *this;
    }


    // 移动赋值运算符
    optional &operator=(optional &&other) noexcept
    {
        if (this != &other)
        {
            if (has_value_)
            {
                value_ref().~T();
            }
            has_value_ = other.has_value_;
            if (has_value_)
            {
                new (storage_) T(std::move(other.value_ref()));
            }
        }
        return *this;
    }

    // 赋值运算符 - 从 nullopt 赋值
    optional &operator=(nullopt_t) noexcept
    {
        reset();
        return *this;
    }

    // 赋值运算符 - 从值赋值
    optional &operator=(const T &value)
    {
        if (has_value_)
        {
            value_ref() = value;
        }
        else
        {
            new (storage_) T(value);
            has_value_ = true;
        }
        return *this;
    }

    optional &operator=(T &&value)
    {
        if (has_value_)
        {
            value_ref() = std::move(value);
        }
        else
        {
            new (storage_) T(std::move(value));
            has_value_ = true;
        }
        return *this;
    }

    // 检查是否包含值
    constexpr explicit operator bool() const noexcept
    {
        return has_value_;
    }

    constexpr bool has_value() const noexcept
    {
        return has_value_;
    }

    // 访问值 - 不检查是否存在
    T &operator*() noexcept
    {
        return value_ref();
    }

    const T &operator*() const noexcept
    {
        return value_ref();
    }

    T *operator->() noexcept
    {
        return &value_ref();
    }

    const T *operator->() const noexcept
    {
        return &value_ref();
    }

    // 访问值 - 检查是否存在，不存在则抛出异常
    T &value()
    {
        if (!has_value_)
        {
            throw bad_optional_access();
        }
        return value_ref();
    }

    const T &value() const
    {
        if (!has_value_)
        {
            throw bad_optional_access();
        }
        return value_ref();
    }

    // 访问值，不存在时返回默认值
    template <typename U>
    T value_or(U &&default_value) const &
    {
        return has_value_ ? value_ref() : static_cast<T>(std::forward<U>(default_value));
    }

    template <typename U>
    T value_or(U &&default_value) &&
    {
        return has_value_ ? std::move(value_ref()) : static_cast<T>(std::forward<U>(default_value));
    }

    // 重置为空状态
    void reset() noexcept
    {
        if (has_value_)
        {
            value_ref().~T();
            has_value_ = false;
        }
    }

    // 就地构造
    template <typename... Args>
    void emplace(Args &&...args)
    {
        if (has_value_)
        {
            value_ref().~T();
        }
        new (storage_) T(std::forward<Args>(args)...);
        has_value_ = true;
    }
};

// 辅助函数：创建 optional
template <typename T>
optional<typename std::decay<T>::type> make_optional(T &&value)
{
    return optional<typename std::decay<T>::type>(std::forward<T>(value));
}

// 比较运算符 - optional 与 optional
template <typename T>
bool operator==(const optional<T> &lhs, const optional<T> &rhs)
{
    if (lhs.has_value() != rhs.has_value())
    {
        return false;
    }
    if (!lhs.has_value())
    {
        return true;
    }
    return *lhs == *rhs;
}

template <typename T>
bool operator!=(const optional<T> &lhs, const optional<T> &rhs)
{
    return !(lhs == rhs);
}

template <typename T>
bool operator<(const optional<T> &lhs, const optional<T> &rhs)
{
    if (!rhs.has_value())
    {
        return false;
    }
    if (!lhs.has_value())
    {
        return true;
    }
    return *lhs < *rhs;
}

// 比较运算符 - optional 与 nullopt
template <typename T>
bool operator==(const optional<T> &lhs, nullopt_t) noexcept
{
    return !lhs.has_value();
}

template <typename T>
bool operator==(nullopt_t, const optional<T> &rhs) noexcept
{
    return !rhs.has_value();
}

template <typename T>
bool operator!=(const optional<T> &lhs, nullopt_t) noexcept
{
    return lhs.has_value();
}

template <typename T>
bool operator!=(nullopt_t, const optional<T> &rhs) noexcept
{
    return rhs.has_value();
}

template <typename T>
bool operator<(const optional<T> &lhs, nullopt_t) noexcept
{
    return false;
}

template <typename T>
bool operator<(nullopt_t, const optional<T> &rhs) noexcept
{
    return rhs.has_value();
}

template <typename T>
bool operator<=(const optional<T> &lhs, nullopt_t) noexcept
{
    return !lhs.has_value();
}

template <typename T>
bool operator<=(nullopt_t, const optional<T> &rhs) noexcept
{
    return true;
}

template <typename T>
bool operator>(const optional<T> &lhs, nullopt_t) noexcept
{
    return lhs.has_value();
}

template <typename T>
bool operator>(nullopt_t, const optional<T> &rhs) noexcept
{
    return false;
}

template <typename T>
bool operator>=(const optional<T> &lhs, nullopt_t) noexcept
{
    return true;
}

template <typename T>
bool operator>=(nullopt_t, const optional<T> &rhs) noexcept
{
    return !rhs.has_value();
}

#endif

} // namespace util


/* ======================= string ======================== */

namespace util
{

template<typename... Args>
inline std::string format(const std::string& fmt, const Args&... args) {
    char buffer[2048];
    std::sprintf(buffer, fmt.c_str(), args...);
    return std::string(buffer);
}

inline std::wstring fromUtf8(const std::string & str) {
#ifdef _WIN32
    if (str.empty()) return std::wstring();
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (wlen <= 0) return std::wstring();
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], wlen);
    return wstr;
#elif defined(__linux__)
    // On Linux, convert UTF-8 to wide string using iconv
    if (str.empty()) return std::wstring();
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1) return std::wstring();

    size_t in_left = str.size();
    const char* in_ptr = str.data();
    std::wstring wstr;
    wchar_t out_buf[4096];
    char* out_ptr = (char*)out_buf;
    size_t out_left = sizeof(out_buf);

    // Reset the conversion state
    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t result = iconv(cd, &in_ptr, &in_left, &out_ptr, &out_left);
        if (result == (size_t)-1) {
            // Handle error or skip invalid sequence
            break;
        }
        size_t converted = (out_ptr - (char*)out_buf) / sizeof(wchar_t);
        wstr.append((const wchar_t*)out_buf, converted);
        out_ptr = (char*)out_buf;
        out_left = sizeof(out_buf);
    }
    iconv_close(cd);
    return wstr;
#else
    // Other platforms: assume no wide string conversion available
    return std::wstring();
#endif
}

inline std::string toUtf8(const std::wstring & str) {
#ifdef _WIN32
    if (str.empty()) return std::string();
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string result(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], len, nullptr, nullptr);
    return result;
#elif defined(__linux__)
    if (str.empty()) return std::string();
    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
    if (cd == (iconv_t)-1) return std::string();

    size_t in_left = str.size() * sizeof(wchar_t);
    const char* in_ptr = reinterpret_cast<const char*>(str.data());
    char out_buf[4096];
    char* out_ptr = out_buf;
    size_t out_left = sizeof(out_buf);
    std::string result;

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t res = iconv(cd, const_cast<char**>(&in_ptr), &in_left, &out_ptr, &out_left);
        if (res == (size_t)-1) break;
        result.append(out_buf, out_ptr - out_buf);
        out_ptr = out_buf;
        out_left = sizeof(out_buf);
    }
    if (out_ptr != out_buf) {
        result.append(out_buf, out_ptr - out_buf);
    }
    iconv_close(cd);
    return result;
#else
    return std::string();
#endif
}

inline std::wstring fromAnsi(const std::string & str) {
#ifdef _WIN32
    if (str.empty()) return std::wstring();
    int wlen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (wlen <= 0) return std::wstring();
    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstr[0], wlen);
    return wstr;
#elif defined(__linux__)
    if (str.empty()) return std::wstring();
    iconv_t cd = iconv_open("WCHAR_T", "GBK//IGNORE");
    if (cd == (iconv_t)-1) return std::wstring();

    size_t in_left = str.size();
    const char* in_ptr = str.data();
    std::wstring wstr;
    wchar_t out_buf[4096];
    char* out_ptr = (char*)out_buf;
    size_t out_left = sizeof(out_buf);

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t result = iconv(cd, (char**)&in_ptr, &in_left, &out_ptr, &out_left);
        if (result == (size_t)-1) {
            break;
        }
        size_t converted = (out_ptr - (char*)out_buf) / sizeof(wchar_t);
        wstr.append((const wchar_t*)out_buf, converted);
        out_ptr = (char*)out_buf;
        out_left = sizeof(out_buf);
    }
    iconv_close(cd);
    return wstr;
#else
    return std::wstring();
#endif
}

inline std::string toAnsi(const std::wstring & str){
#ifdef _WIN32
    if (str.empty()) return std::string();
    int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return std::string();
    std::string result(len, 0);
    WideCharToMultiByte(CP_ACP, 0, str.c_str(), (int)str.size(), &result[0], len, nullptr, nullptr);
    return result;
#elif defined(__linux__)
    if (str.empty()) return std::string();
    iconv_t cd = iconv_open("GBK//IGNORE", "WCHAR_T");
    if (cd == (iconv_t)-1) return std::string();

    size_t in_left = str.size() * sizeof(wchar_t);
    const char* in_ptr = reinterpret_cast<const char*>(str.data());
    char out_buf[4096];
    char* out_ptr = out_buf;
    size_t out_left = sizeof(out_buf);
    std::string result;

    iconv(cd, nullptr, nullptr, nullptr, nullptr);

    while (in_left > 0) {
        size_t res = iconv(cd, const_cast<char**>(&in_ptr), &in_left, &out_ptr, &out_left);
        if (res == (size_t)-1) break;
        result.append(out_buf, out_ptr - out_buf);
        out_ptr = out_buf;
        out_left = sizeof(out_buf);
    }
    if (out_ptr != out_buf) {
        result.append(out_buf, out_ptr - out_buf);
    }
    iconv_close(cd);
    return result;
#else
    return std::string();
#endif
}

inline std::string ansi2utf8(const std::string & str){
    return toUtf8(fromAnsi(str));
}

inline std::string utf82ansi(const std::string & str){
    return toAnsi(fromUtf8(str));
}

inline std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

inline bool startWith(const std::string& str, const std::string& prefix, bool caseSensitive = true) {
    if (prefix.empty()) return false;
    if (str.length() < prefix.length()) return false;

    std::string cmpStr = str.substr(0, prefix.length());
    std::string cmpPrefix = prefix;

    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
        cmpPrefix = toLower(cmpPrefix);
    }

    return (cmpStr == cmpPrefix);
}

inline bool startWith(const std::string& str, const std::regex& regex, bool caseSensitive) {
    std::string cmpStr = str;
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
    }
    std::smatch match;
    return std::regex_search(cmpStr, match, regex) && match.position() == 0;
}

inline bool endWith(const std::string& str, const std::string& suffix, bool caseSensitive = true) {
    if (suffix.empty()) return false;
    if (str.length() < suffix.length()) return false;
    
    std::string cmpStr = str.substr(str.length() - suffix.length());
    std::string cmpSuffix = suffix;
    
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
        cmpSuffix = toLower(cmpSuffix);
    }

    return (cmpStr == cmpSuffix);
}

inline bool endWith(const std::string& str, const std::regex& regex, bool caseSensitive = true) {
    std::string cmpStr = str;
    if (!caseSensitive) {
        cmpStr = toLower(cmpStr);
    }
    std::smatch match;
    // Search for the regex anywhere in the string
    auto searchStart = cmpStr.cbegin();
    while (std::regex_search(searchStart, cmpStr.cend(), match, regex)) {
        // Check if the match ends at the end of the string
        if ((match.position() + match.length()) == cmpStr.length()) {
            return true;
        }
        // Move search start forward to avoid infinite loop
        searchStart = match.suffix().first;
    }
    return false;
}


inline bool contain(const std::string& str, const std::string & target, bool caseSensitive = true) {
    if (target.empty()) return false;
    if (caseSensitive) {
        return str.find(target) != std::string::npos;
    } else {
        std::string lowerStr = toLower(str);
        std::string lowerTarget = toLower(target);
        return lowerStr.find(lowerTarget) != std::string::npos;
    }
}

inline bool contain(const std::string& str, const std::regex & regex) {
    return std::regex_search(str, regex);
}

inline std::string removeSuffix(const std::string& str, const std::string & suffix, bool caseSensitive = true){
    if (endWith(str, suffix, caseSensitive)){
        return str.substr(0, str.length() - suffix.length());
    }else {
        return str;
    }
}

inline std::string removePrefix(const std::string& str, const std::string & prefix, bool caseSensitive = true){
    if (startWith(str, prefix, caseSensitive)){
        return str.substr(prefix.size(), str.length() - prefix.length());
    }else {
        return str;
    }
}

inline std::string repleace(const std::string& str, const std::string& from, const std::string& to, bool all = false) {
    if (from.empty()) return str;
    std::string result = str;
    size_t pos = 0;
    if (all) {
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
    } else {
        pos = result.find(from);
        if (pos != std::string::npos) {
            result.replace(pos, from.length(), to);
        }
    }
    return result;
}

inline std::string repleace(const std::string& str, const std::regex& regex, const std::string& to, bool all = false) {
    if (all) {
        return std::regex_replace(str, regex, to);
    } else {
        std::smatch match;
        if (std::regex_search(str, match, regex)) {
            std::string result = str;
            result.replace(match.position(), match.length(), to);
            return result;
        }
        return str;
    }
}

inline std::vector<std::string> split(const std::string& str, const std::string& delimiter, bool caseSensitive = false) {
    std::vector<std::string> tokens;
    if (delimiter.empty()) {
        tokens.push_back(str);
        return tokens;
    }

    std::string::size_type start = 0;
    std::string tempStr = caseSensitive ? str : toLower(str);
    std::string tempDelim = caseSensitive ? delimiter : toLower(delimiter);

    while (true) {
        std::string::size_type end = tempStr.find(tempDelim, start);
        if (end == std::string::npos) {
            tokens.push_back(str.substr(start));
            break;
        }
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    return tokens;
}

inline std::vector<std::string> split(const std::string& str, const std::regex& regex, bool caseSensitive = false) {
    std::vector<std::string> tokens;
    std::string s = caseSensitive ? str : toLower(str);
    std::sregex_token_iterator it(s.begin(), s.end(), regex, -1);
    std::sregex_token_iterator end;
    for (; it != end; ++it) {
        tokens.push_back(it->str());
    }
    return tokens;
}

inline std::string ltrim(const std::string& str){
    auto it = std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); });
    return std::string(it, str.end());
}

inline std::string rtrim(const std::string& str){
    auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); });
    
    return std::string(str.begin(), it.base());
}

inline std::string trims(const std::string& str) { return ltrim(rtrim(str));}

template<typename Number>
inline bool toNumber(const std::string& str, Number& value) {
    try {
        size_t idx = 0;
        if (typeid(Number) == typeid(int)) {
            value = static_cast<Number>(std::stoi(str, &idx));
        } else if (typeid(Number) == typeid(size_t)) {
            value = static_cast<Number>(std::stoull(str, &idx));
        } else if (typeid(Number) == typeid(uint32_t)) {
            value = static_cast<Number>(std::stoul(str, &idx));
        } else if (typeid(Number) == typeid(int32_t)) {
            value = static_cast<Number>(std::stol(str, &idx));
        } else if (typeid(Number) == typeid(double)) {
            value = static_cast<Number>(std::stod(str, &idx));
        } else if (typeid(Number) == typeid(float)) {
            value = static_cast<Number>(std::stof(str, &idx));
        } else if (typeid(Number) == typeid(long)) {
            value = static_cast<Number>(std::stol(str, &idx));
        } else if (typeid(Number) == typeid(unsigned long)) {
            value = static_cast<Number>(std::stoul(str, &idx));
        } else if (typeid(Number) == typeid(long long)) {
            value = static_cast<Number>(std::stoll(str, &idx));
        } else if (typeid(Number) == typeid(unsigned long long)) {
            value = static_cast<Number>(std::stoull(str, &idx));
        } else {
            std::istringstream iss(str);
            iss >> value;
            return !iss.fail() && iss.eof();
        }
        return idx == str.size();
    } catch (...) {
        return false;
    }
}

inline std::string dumpBinary(const uint8_t * pointer, size_t uLen){
    size_t cursor = 0;
    std::ostringstream os;
    while (cursor < uLen)
    {
        int i;
        int thisline = (std::min)(uLen - cursor, 16ULL);

        os << std::setw(8) << std::setfill('0') << std::hex << cursor << " ";

        for (i = 0; i < thisline; i++)
        {
            os << std::setw(2) << std::setfill('0') << std::hex << (uint32_t)pointer[i] << " "; 
        }

        for (; i < 16; i++)
        {
            os << "   ";
        }

        for (i = 0; i < thisline; i++)
        {
            if(pointer[i] >= 0x20 && pointer[i] < 0x7f){
                os << pointer[i];
            }else{
                os << '.';
            }
        }

        os << std::endl;
        cursor += thisline;
        pointer += thisline;
    }

    return os.str();
}

inline std::string dumpBinary(const char * pointer, size_t uLen){
    return dumpBinary(reinterpret_cast<const uint8_t*>(pointer), uLen);
}

} // namespace util


/* ======================== data structure ============================= */

namespace util
{

template <typename T>
class SafeLinkQueue
{
public:
    struct Node
    {
        T data;
        std::unique_ptr<Node> next;
    };

public:
    // NOTE - 提前创建一个空节点，用来标记队列为空
    SafeLinkQueue() : m_head(new Node()), m_pTail(m_head.get()),m_size{0} {}
    ~SafeLinkQueue() = default;

    SafeLinkQueue(const SafeLinkQueue &other) = delete;
    SafeLinkQueue &operator=(const SafeLinkQueue &other) = delete;

    size_t size(){
        return m_size.load();
    }

    optional<T> tryPop()
    {
        std::unique_ptr<Node> old_head;
        {
            std::lock_guard<std::mutex> head_lock(m_mutHead);
            if (m_head.get() == tail())
            {
                return nullopt;
            }
            old_head = std::move(pop());
        }
        return old_head ? old_head->data : optional<T>();
    }

    T popWait()
    {
        std::unique_lock<std::mutex> head_lock(m_mutHead);

        m_condData.wait(head_lock, [&]{ return m_head.get() != tail(); });

        std::unique_ptr<Node> old_head = pop();
        return old_head->data;
    }

    T popWait(std::chrono::microseconds timeout)
    {
        std::unique_lock<std::mutex> head_lock(m_mutHead);

        auto status = m_condData.wait_for(head_lock, timeout, [&]{ return m_head.get() != tail(); });
        if (!status) {
            throw util::TimeoutError("queue pop wait timeout");
        }

        std::unique_ptr<Node> old_head = pop();
        return old_head->data;
    }

    template<class U>
    void push(U && data)
    {
        std::unique_ptr<Node> p(new Node());
        auto pData = p.get();

        {
            std::lock_guard<std::mutex> tail_lock(m_mutTail);
            m_pTail->data = std::forward<U>(data);
            m_pTail->next = std::move(p);
            m_pTail = pData;
            ++m_size;
        }

        m_condData.notify_one();
    }

    void clear()
    {
        std::lock(m_mutHead, m_mutTail);
        std::lock_guard<std::mutex> head_lock(m_mutHead, std::adopt_lock);
        std::lock_guard<std::mutex> tail_lock(m_mutTail, std::adopt_lock);

        m_head.reset(new Node());
        m_pTail = m_head.get();
        m_size.store(0);
    }

private:
    const Node* tail()
    {
        std::lock_guard<std::mutex> tail_lock(m_mutTail);
        return m_pTail;
    }

    std::unique_ptr<Node> pop(){
        --m_size;
        std::unique_ptr<Node> old_head = std::move(m_head);
        m_head = std::move(old_head->next);
        return old_head;  // NRVO or move will be applied automatically
    }

private:
    std::atomic_size_t m_size;

    std::mutex m_mutHead;
    std::unique_ptr<Node> m_head;

    std::mutex m_mutTail;
    // NOTE - 由于使用的是 unique_ptr 来管理节点，一个对象只能放入一个 unique_ptr 中，因此队尾就直接使用指针
    Node *m_pTail;

    std::condition_variable m_condData;
};
} // namespace util


#endif /* UTILS_HPP */
