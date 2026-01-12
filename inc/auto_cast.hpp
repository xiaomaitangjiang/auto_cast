#pragma once
#include <cassert>
#include <optional>
#include <type_traits>
#include <typeinfo>

// 类型特征检查辅助模板
template<typename T>
struct is_pointer_like {
    static constexpr bool value = false;
};

template<typename T>
struct is_pointer_like<T*> {
    static constexpr bool value = true;
};

template<typename T>
struct is_pointer_like<T* const> {
    static constexpr bool value = true;
};

template<typename T>
struct is_pointer_like<T* volatile> {
    static constexpr bool value = true;
};

template<typename T>
struct is_pointer_like<T* const volatile> {
    static constexpr bool value = true;
};

template<typename T>
constexpr bool is_pointer_like_v=is_pointer_like<T>::value;


// 引用类型的检查
template<typename T>
struct is_reference_like {
    static constexpr bool value = false;
};

template<typename T>
struct is_reference_like<T&> {
    static constexpr bool value = true;
};

template<typename T>
struct is_reference_like<T&&> {
    static constexpr bool value = true;
};

template<typename T>
constexpr bool is_reference_like_v =is_reference_like<T>::value;

// 策略标签
struct safe_cast_tag {};      // 安全模式，禁止reinterpret_cast
struct unsafe_cast_tag {};    // 不安全模式，允许reinterpret_cast
struct strict_cast_tag {};    // 严格模式，禁止某些转换

// 默认模式：安全模式
struct default_policy {
    using tag = safe_cast_tag;
    static constexpr bool allow_reinterpret = false;
    static constexpr bool allow_const_removal = true;
    static constexpr bool allow_non_polymorphic_downcast = true;
};

// 不安全模式策略
struct unsafe_policy {
    using tag = unsafe_cast_tag;
    static constexpr bool allow_reinterpret = true;
    static constexpr bool allow_const_removal = true;
    static constexpr bool allow_non_polymorphic_downcast = true;
};

// 严格模式策略
struct strict_policy {
    using tag = strict_cast_tag;
    static constexpr bool allow_reinterpret = false;
    static constexpr bool allow_const_removal = false;  // 禁止去const
    static constexpr bool allow_non_polymorphic_downcast = false;  // 禁止非多态向下转换
};

// 主模板，带策略参数
template<typename To, typename From, typename Policy = default_policy>
class auto_cast_impl {
private:
    // 使用策略标志
    static constexpr bool allow_reinterpret = Policy::allow_reinterpret;
    static constexpr bool allow_const_removal = Policy::allow_const_removal;
    static constexpr bool allow_non_polymorphic_downcast = Policy::allow_non_polymorphic_downcast;
    
    // 编译时错误信息
    template<bool Condition>
    struct static_check {
        static_assert(Condition, 
            "auto_cast<>: This conversion is not allowed by the current policy");
    };
    
    // 编译时错误信息（特定于reinterpret_cast）
    struct reinterpret_not_allowed {
        static_assert(allow_reinterpret, 
            "auto_cast<>: reinterpret_cast is not allowed by the current policy. "
            "Consider using auto_cast<To, unsafe_policy> or auto_cast_unsafe<To>.");
    };
    
    // 编译时错误信息（特定于去const）
    struct const_removal_not_allowed {
        static_assert(allow_const_removal,
            "auto_cast<>: Removing const qualification is not allowed by the current policy. "
            "Consider using auto_cast<To, default_policy> instead of auto_cast<To, strict_policy>.");
    };
    
    // 编译时错误信息（特定于非多态向下转换）
    struct non_polymorphic_downcast_not_allowed {
        static_assert(allow_non_polymorphic_downcast,
            "auto_cast<>: Non-polymorphic downcast is not allowed by the current policy. "
            "Consider using auto_cast<To, default_policy> instead of auto_cast<To, strict_policy>.");
    };

    // 基本类型转换实现
    template<typename T = To, typename F = From,
              std::enable_if_t<std::is_same_v<T, F>, int> = 0>
    static T same_type_cast(F from) noexcept {
        return from;
    }
    
    // 去const转换（检查策略）
    template<typename T = To, typename F = From,
             typename std::enable_if_t<
                 std::is_same_v<typename std::remove_const_t<T>,
                     typename std::remove_const_t<F>> && 
                 !std::is_same_v<T, F> &&
                 !static_cast<bool>(std::is_const_v<T>) &&
                 static_cast<bool>(std::is_const_v<F>)>>
    static T remove_const_cast(F from) noexcept {
        static_check<allow_const_removal> check;
        return const_cast<T>(from);
    }
    
    // 向上转换
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 std::is_base_of_v<T, F> && 
                 (is_pointer_like_v<T> && is_pointer_like_v<F>), int> = 0>
    static T up_cast(F from) noexcept {
        return static_cast<T>(from);
    }
    
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 std::is_base_of_v<T, F> && 
                 (is_reference_like<T>::value && is_reference_like<F>::value), int> = 0>
    static T up_cast(F from) noexcept {
        return static_cast<T>(from);
    }
    
    // 向下转换 - 多态类型用dynamic_cast
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 std::is_base_of_v<F, T> && 
                 std::is_polymorphic_v<F> &&
                 (is_pointer_like_v<T> && is_pointer_like_v<F>), int> = 0>
    static T safe_down_cast(F from) {
        auto result = dynamic_cast<T>(from);
        if (!result && from) {
            throw std::bad_cast();
        }
        return result;
    }
    
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 std::is_base_of_v<F, T> && 
                 std::is_polymorphic_v<F> &&
                 (is_reference_like_v<T> && is_reference_like_v<F>), int> = 0>
    static T safe_down_cast(F from) {
        using raw_to =  std::remove_reference_t<T>;
        using raw_from =  std::remove_reference_t<F>;
        auto* ptr = dynamic_cast<raw_to*>(&from);
        if (!ptr) {
            throw std::bad_cast();
        }
        return static_cast<T>(*ptr);
    }
    
    // 向下转换 - 非多态类型用static_cast（检查策略）
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 std::is_base_of_v<F, T> && 
                 !std::is_polymorphic_v<F> &&
                 (is_pointer_like_v<T> || is_reference_like_v<T>) &&
                 (is_pointer_like_v<F> || is_reference_like_v<F>), int> = 0>
    static T unsafe_down_cast(F from) noexcept {
        static_check<allow_non_polymorphic_downcast> check;
        return static_cast<T>(from);
    }
    
    // 标准转换
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 !std::is_same_v<T, F> &&
                 std::is_convertible_v<F, T> &&
                 !std::is_base_of_v<T, F> &&
                 !std::is_base_of_v<F, T> &&
                 !std::is_const_v<F>, int> = 0>
    static T standard_cast(F from) noexcept(noexcept(static_cast<T>(from))) {
        return static_cast<T>(from);
    }
    
    // 强制重新解释转换（检查策略）
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 allow_reinterpret &&
                 ((is_pointer_like_v<T> && is_pointer_like_v<F>) ||
                 (std::is_integral_v<T> && is_pointer_like_v<F>) ||
                 (is_pointer_like_v<T> && std::is_integral_v<F>)), int> = 0>
    static T reinterpret_cast_impl(F from) noexcept {
        return reinterpret_cast<T>(from);
    }
    
    // reinterpret_cast不允许的版本
    template<typename T = To, typename F = From,
              std::enable_if_t<
                 !allow_reinterpret &&
                 ((is_pointer_like<T>::value && is_pointer_like<F>::value) ||
                 (std::is_integral_v<T> && is_pointer_like<F>::value) ||
                 (is_pointer_like<T>::value && std::is_integral_v<F>)), int> = 0>
    static T reinterpret_cast_impl(F /*unused*/) noexcept {
        // 触发编译时错误
        reinterpret_not_allowed error;
        return T{};
    }
    
public:
    static To cast(From from) {
        // 按优先级尝试不同的转换
        if constexpr (std::is_same_v<To, From>) {
            return same_type_cast(from);
        }
        else if constexpr (std::is_same_v<
             std::remove_const_t<To>,
             std::remove_const_t<From>
        > && !std::is_same_v<To, From>) {
            // 只有const不同的类型
            if constexpr (std::is_const_v<From> && !std::is_const_v<To>) {
                return remove_const_cast(from);
            } else {
                return static_cast<To>(from);
            }
        }
        else if constexpr (std::is_base_of_v<To, From>) {
            // 向上转换总是安全的
            return up_cast(from);
        }
        else if constexpr (std::is_base_of_v<From, To>) {
            // 向下转换
            if constexpr (std::is_polymorphic_v<From>) {
                return safe_down_cast(from);
            } else {
                return unsafe_down_cast(from);
            }
        }
        else if constexpr (std::is_convertible_v<From, To>) {
            // 标准转换
            return standard_cast(from);
        }
        else if constexpr ((is_pointer_like<To>::value && is_pointer_like<From>::value) ||
                          (std::is_integral_v<To> && is_pointer_like<From>::value) ||
                          (is_pointer_like<To>::value && std::is_integral_v<From>)) {
            // 强制重新解释转换
            return reinterpret_cast_impl(from);
        }
        else {
            static_assert(sizeof(From) == 0, 
                "auto_cast<>: No suitable conversion found between types");
        }
    }
};

// 用户接口 - 带策略模板参数
template<typename To,typename Policy = default_policy,typename From >
To auto_cast(From from) {
    return auto_cast_impl<To, From, Policy>::cast(from);
}

// 便捷别名
template<typename To, typename From>
To auto_cast_safe(From from) {
    return auto_cast_impl<To, From, default_policy>::cast(from);
}

template<typename To, typename From>
To auto_cast_unsafe(From from) {
    return auto_cast_impl<To, From, unsafe_policy>::cast(from);
}

template<typename To, typename From>
To auto_cast_strict(From from) {
    return auto_cast_impl<To, From, strict_policy>::cast(from);
}


//运行时检查版本,返回std::optional
template<typename To,  typename From ,typename Policy = default_policy>
std::optional<To> try_auto_cast(From from) noexcept {
    try {
        return auto_cast_impl<To, From, Policy>::cast(from);
    } catch (const std::bad_cast&) {
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}