#ifndef UTILOPTIONAL_HPP
#define UTILOPTIONAL_HPP

#if __cplusplus >= 201703L
#include <optional>
#endif

#include "utilException.hpp"

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


#endif /* UTILOPTIONAL_HPP */
