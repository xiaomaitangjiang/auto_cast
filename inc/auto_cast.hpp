#pragma once
#include <cassert>
#include <concepts>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <typeinfo>

#define CPP_20 __cplusplus >= 202002
#define CPP_17 __cplusplus >= 201703
#define CPP_14 __cplusplus >= 201402
#define CPP_11 __cplusplus >= 201103
/*
/ 模板工具
/ 根据cpp版本选择使用的模板
*/

#if CPP_20
// 指针类型检查
template <typename T>
concept is_pointer_like = std::is_pointer_v<T>;

template <typename T>
constexpr bool is_pointer_like_v = is_pointer_like<T>;

// 引用类型的检查
template <typename T>
concept is_reference_like = std::is_reference_v<T>;

template <typename T>
constexpr bool is_reference_like_v = is_reference_like<T>;

template <typename T>
concept has_policy_bool = requires(T policy) {
  requires std::same_as<decltype(policy.allow_reinterpret), const bool>;
  requires std::same_as<decltype(policy.allow_const_removal), const bool>;
  requires std::same_as<decltype(policy.allow_non_polymorphic_downcast),
                        const bool>;
  requires std::same_as<decltype(policy.allow_standard_pointer_integer_cast),
                        const bool>;
};

template <typename T>
concept is_cast_policy = requires {
  typename T::tag;
  requires has_policy_bool<T>;
};

template <typename T>
concept standard_pointer_integer =
    std::is_same_v<T, std::intptr_t> || std::is_same_v<T, std::uintptr_t>;

template <typename From, typename To>
concept PointerToStandardInteger =
    std::is_pointer_v<From> && standard_pointer_integer<To>;

template <typename From, typename To>
concept StandardIntegerToPointer =
    standard_pointer_integer<From> && std::is_pointer_v<To>;

template <typename From, typename To>
concept is_standard_pointer_integer_conversion =
    PointerToStandardInteger<From, To> || StandardIntegerToPointer<From, To>;

#elif CPP_17

// 指针类型检查
template <typename T>
struct is_pointer_like
{
  static constexpr bool value = false;
};

template <typename T>
struct is_pointer_like<T*>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* const>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* volatile>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* const volatile>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_pointer_like_v = is_pointer_like<T>::value;

// 引用类型的检查
template <typename T>
struct is_reference_like
{
  static constexpr bool value = false;
};

template <typename T>
struct is_reference_like<T&>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_reference_like<T&&>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_reference_like_v = is_reference_like<T>::value;

template <typename T, typename = void>
struct is_cast_policy : std::false_type
{
};

template <typename T>
struct is_cast_policy<
    T, std::void_t<typename T::tag, typename T::allow_reinterpret,
                   typename T::allow_const_removal,
                   typename T::allow_non_polymorphic_downcast,
                   typename T::allow_standard_pointer_integer_cast>>
    : std::true_type
{
};

template <typename T>
constexpr bool standard_pointer_integer =
    std::is_same_v<T, std::intptr_t> || std::is_same_v<T, std::uintptr_t>;

template <typename From, typename To>
constexpr bool PointerToStandardInteger =
    std::is_pointer_v<From> && standard_pointer_integer<To>;

template <typename From, typename To>
constexpr bool StandardIntegerToPointer =
    standard_pointer_integer<From> && std::is_pointer_v<To>;

template <typename From, typename To>
constexpr bool is_standard_pointer_integer_conversion =
    PointerToStandardInteger<From, To> || StandardIntegerToPointer<From, To>;

#elif CPP_14
// 指针类型检查
template <typename T>
struct is_pointer_like
{
  static constexpr bool value = false;
};

template <typename T>
struct is_pointer_like<T*>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* const>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* volatile>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_pointer_like<T* const volatile>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_pointer_like_v = is_pointer_like<T>::value;

// 引用类型的检查
template <typename T>
struct is_reference_like
{
  static constexpr bool value = false;
};

template <typename T>
struct is_reference_like<T&>
{
  static constexpr bool value = true;
};

template <typename T>
struct is_reference_like<T&&>
{
  static constexpr bool value = true;
};

template <typename T>
constexpr bool is_reference_like_v = is_reference_like<T>::value;

template <typename T, typename = void>
struct is_cast_policy : std::false_type
{
};

template <typename T>
struct is_cast_policy<
    T, std::void_t<typename T::tag, typename T::allow_reinterpret,
                   typename T::allow_const_removal,
                   typename T::allow_non_polymorphic_downcast,
                   typename T::allow_standard_pointer_integer_cast>>
    : std::true_type
{
};

template <typename T>
constexpr bool standard_pointer_integer =
    std::is_same_v<T, std::intptr_t> || std::is_same_v<T, std::uintptr_t>;

template <typename From, typename To>
constexpr bool PointerToStandardInteger =
    std::is_pointer_v<From> && standard_pointer_integer<To>;

template <typename From, typename To>
constexpr bool StandardIntegerToPointer =
    standard_pointer_integer<From> && std::is_pointer_v<To>;

template <typename From, typename To>
constexpr bool is_standard_pointer_integer_conversion =
    PointerToStandardInteger<From, To> || StandardIntegerToPointer<From, To>;

#endif

// 策略标签
struct safe_cast_tag
{
};  // 安全模式，禁止reinterpret_cast
struct unsafe_cast_tag
{
};  // 不安全模式，允许reinterpret_cast
struct strict_cast_tag
{
};  // 严格模式，禁止某些转换

// 默认模式：安全模式
struct default_policy
{
  using tag = safe_cast_tag;
  static constexpr bool allow_reinterpret = false;
  static constexpr bool allow_const_removal = true;
  static constexpr bool allow_non_polymorphic_downcast = false;
  static constexpr bool allow_standard_pointer_integer_cast = true;
};

// 不安全模式策略
struct unsafe_policy
{
  using tag = unsafe_cast_tag;
  static constexpr bool allow_reinterpret = true;
  static constexpr bool allow_const_removal = true;
  static constexpr bool allow_non_polymorphic_downcast = true;
  static constexpr bool allow_standard_pointer_integer_cast = true;
};

// 严格模式策略
struct strict_policy
{
  using tag = strict_cast_tag;
  static constexpr bool allow_reinterpret = false;
  static constexpr bool allow_const_removal = false;  // 禁止去const
  static constexpr bool allow_non_polymorphic_downcast =
      false;  // 禁止非多态向下转换
  static constexpr bool allow_standard_pointer_integer_cast =
      false;  // 禁止指针和整数相互转换
};

#if __cplusplus >= 202002

template <typename To, typename From, is_cast_policy Policy = default_policy>
class auto_cast_impl
{
private:
  // 使用策略标志
  static constexpr bool allow_reinterpret = Policy::allow_reinterpret;
  static constexpr bool allow_const_removal = Policy::allow_const_removal;
  static constexpr bool allow_non_polymorphic_downcast =
      Policy::allow_non_polymorphic_downcast;
  static constexpr bool allow_standard_pointer_integer_cast =
      Policy::allow_standard_pointer_integer_cast;

  // 编译时错误信息
  template <bool Condition>
  struct static_check
  {
    static_assert(
        Condition,
        "auto_cast<>: This conversion is not allowed by the current policy");
  };

  // 编译时错误信息（特定于reinterpret_cast）
  struct reinterpret_not_allowed
  {
    static_assert(
        allow_reinterpret,
        "auto_cast<>: reinterpret_cast is not allowed by the current policy. "
        "Consider using auto_cast<To, unsafe_policy> or auto_cast_unsafe<To>.");
  };

  // 编译时错误信息（特定于去const）
  struct const_removal_not_allowed
  {
    static_assert(allow_const_removal,
                  "auto_cast<>: Removing const qualification is not allowed by "
                  "the current policy. "
                  "Consider using auto_cast<To, default_policy> instead of "
                  "auto_cast<To, strict_policy>.");
  };

  // 编译时错误信息（特定于非多态向下转换）
  struct non_polymorphic_downcast_not_allowed
  {
    static_assert(allow_non_polymorphic_downcast,
                  "auto_cast<>: Non-polymorphic downcast is not allowed by the "
                  "current policy. "
                  "Consider using auto_cast<To, default_policy> instead of "
                  "auto_cast<To, strict_policy>.");
  };

  struct standard_pointer_integer_cast_not_allowed
  {
    static_assert(
        allow_standard_pointer_integer_cast,
        "auto_cast<>: standard pointer integer cast is not allowed by the "
        "current policy. "
        "Consider using auto_cast<To, default_policy> instead of "
        "auto_cast<To, strict_policy>.");
  };

  // 基本类型转换实现
  template <typename T = To, typename F = From>
  static T same_type_cast(F from) noexcept
    requires(std::is_same_v<T, F>)
  {
    return from;
  }

  // 去const转换（检查策略）
  template <typename T = To, typename F = From>
  static T remove_const_cast(F from) noexcept
    requires(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<F>> &&
             !std::is_same_v<T, F> && !static_cast<bool>(std::is_const_v<T>) &&
             static_cast<bool>(std::is_const_v<F>))
  {
    static_check<allow_const_removal> check;
    return const_cast<T>(from);
  }

  // 向上转换
  template <typename T = To, typename F = From>
  static T up_cast(F from) noexcept
    requires(
        std::is_base_of_v<std::remove_pointer_t<F>, std::remove_pointer_t<T>> &&
        (is_pointer_like_v<T> && is_pointer_like_v<F>))
  {
    return static_cast<T>(from);
  }

  template <typename T = To, typename F = From>
  static T up_cast(F from) noexcept
    requires(std::is_base_of_v<T, F> &&
             (is_reference_like_v<T> && is_reference_like_v<F>))
  {
    return static_cast<T>(from);
  }

  // 向下转换 - 多态类型用dynamic_cast
  template <typename T = To, typename F = From>
  static T safe_down_cast(F from)
    requires(
        std::is_base_of_v<std::remove_pointer_t<F>, std::remove_pointer_t<T>> &&
        std::is_polymorphic_v<std::remove_pointer_t<F>> &&
        (is_pointer_like_v<T> && is_pointer_like_v<F>))
  {
    auto result = dynamic_cast<T>(from);
    if (!result && from) {
      throw std::bad_cast();
    }
    return result;
  }

  template <typename T = To, typename F = From>
  static T safe_down_cast(F from)
    requires(std::is_base_of_v<F, T> && std::is_polymorphic_v<F> &&
             (is_reference_like_v<T> && is_reference_like_v<F>))
  {
    using raw_to = std::remove_reference_t<T>;
    using raw_from = std::remove_reference_t<F>;
    auto* ptr = dynamic_cast<raw_to*>(&from);
    if (!ptr) {
      throw std::bad_cast();
    }
    return static_cast<T>(*ptr);
  }

  // 向下转换 - 非多态类型用static_cast（检查策略）
  template <typename T = To, typename F = From>
  static T unsafe_down_cast(F from) noexcept
    requires(
        std::is_base_of_v<std::remove_pointer_t<F>, std::remove_pointer_t<T>> &&
        !std::is_polymorphic_v<std::remove_pointer_t<F>> &&
        (is_pointer_like_v<T> || is_reference_like_v<T>) &&
        (is_pointer_like_v<F> || is_reference_like_v<F>))
  {
    static_check<allow_non_polymorphic_downcast> check;
    return static_cast<T>(from);
  }

  // 标准转换
  template <typename T = To, typename F = From>
  static T standard_cast(F from) noexcept(noexcept(static_cast<T>(from)))
    requires(!std::is_same_v<T, F> && std::is_convertible_v<F, T> &&
             !std::is_base_of_v<T, F> && !std::is_base_of_v<F, T> &&
             !std::is_const_v<F>)
  {
    return static_cast<T>(from);
  }

  // 强制重新解释转换（检查策略）
  template <typename T = To, typename F = From>
  static T reinterpret_cast_impl(F from) noexcept
    requires(allow_reinterpret &&
             ((is_pointer_like_v<T> && is_pointer_like_v<F>) ||
              (std::is_integral_v<T> && is_pointer_like_v<F>) ||
              (is_pointer_like_v<T> && std::is_integral_v<F>)))
  {
    return reinterpret_cast<T>(from);
  }

  // reinterpret_cast不允许的版本
  template <typename T = To, typename F = From>
  static T reinterpret_cast_impl(F /*unused*/) noexcept
    requires(!allow_reinterpret &&
             ((is_pointer_like_v<T> && is_pointer_like_v<F>) ||
              (std::is_integral_v<T> && is_pointer_like_v<F>) ||
              (is_pointer_like_v<T> && std::is_integral_v<F>)))
  {
    // 触发编译时错误
    reinterpret_not_allowed error;
    return T{};
  }

  template <typename T = To, typename F = From>
  static T standard_pointer_integer_cast(F from) noexcept
    requires(allow_standard_pointer_integer_cast &&
             ((std::is_integral_v<T> && is_pointer_like_v<F>) ||
              (is_pointer_like_v<T> && std::is_integral_v<F>)))
  {
    return reinterpret_cast<T>(from);
  }

  template <typename T = To, typename F = From>
  static T standard_pointer_integer_cast(F /*from*/) noexcept
    requires(!allow_standard_pointer_integer_cast &&
             ((std::is_integral_v<T> && is_pointer_like_v<F>) ||
              (is_pointer_like_v<T> && std::is_integral_v<F>)))
  {
    standard_pointer_integer_cast_not_allowed error;
    return T{};
  }

public:
  static To cast(From from)
  {
    // 按优先级尝试不同的转换
    if constexpr (std::is_same_v<To, From>) {
      return same_type_cast(from);
    }
    else if constexpr (std::is_same_v<std::remove_const_t<To>,
                                      std::remove_const_t<From>>) {
      // 只有const不同的类型
      if constexpr (std::is_const_v<From> && !std::is_const_v<To>) {
        return remove_const_cast(from);
      }
      else {
        return static_cast<To>(from);
      }
    }
    else if constexpr (std::is_base_of_v<std::remove_pointer_t<To>,
                                         std::remove_pointer_t<From>>) {
      // 向上转换总是安全的
      return up_cast(from);
    }
    else if constexpr (std::is_base_of_v<std::remove_pointer_t<From>,
                                         std::remove_pointer_t<To>>) {
      // 向下转换
      if constexpr (std::is_polymorphic_v<std::remove_pointer_t<From>>) {
        return safe_down_cast(from);
      }
      else {
        return unsafe_down_cast(from);
      }
    }
    else if constexpr (std::is_convertible_v<From, To>) {
      // 标准转换
      return standard_cast(from);
    }
    else if constexpr (is_standard_pointer_integer_conversion<From, To>) {
      return standard_pointer_integer_cast(from);
    }
    else if constexpr ((is_pointer_like_v<To> && is_pointer_like_v<From>) ||
                       (std::is_integral_v<To> && is_pointer_like_v<From>) ||
                       (is_pointer_like_v<To> && std::is_integral_v<From>)) {
      // 强制重新解释转换
      return reinterpret_cast_impl(from);
    }
    else {
      static_assert(sizeof(From) == 0,
                    "auto_cast<>: No suitable conversion found between types");
    }
  }
};

#elif CPP_11

// 转换标签
struct same_type_tag
{
};
struct up_cast_tag
{
};
struct down_cast_polymorphic_tag
{
};
struct down_cast_non_polymorphic_tag
{
};
struct const_removal_tag
{
};
struct standard_pointer_integer_tag
{
};
struct generic_pointer_integer_tag
{
};
struct standard_conversion_tag
{
};
struct reinterpret_cast_tag
{
};
struct invalid_cast_tag
{
};

// 类型特征辅助
template <typename T>
struct remove_cv_ptr
{
  using type = std::remove_cv_t<std::remove_pointer_t<T>>;
};

template <typename T>
using remove_cv_ptr_t = typename remove_cv_ptr<T>::type;

// 辅助检查函数
template <typename To, typename From>
constexpr bool is_up_cast_impl()
{
  return std::is_pointer<To>::value && std::is_pointer<From>::value &&
         std::is_base_of<remove_cv_ptr_t<To>, remove_cv_ptr_t<From>>::value;
}

template <typename To, typename From>
constexpr bool is_down_cast_impl()
{
  return std::is_pointer<To>::value && std::is_pointer<From>::value &&
         std::is_base_of<remove_cv_ptr_t<From>, remove_cv_ptr_t<To>>::value;
}

// 递归标签分发器
template <typename To, typename From, typename Policy, int Step = 0>
struct get_cast_tag
{
  using type = typename get_cast_tag<To, From, Policy, Step + 1>::type;
};

// Step 0: 检查相同类型
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 0>
{
  using type = typename std::conditional<
      std::is_same<To, From>::value, same_type_tag,
      typename get_cast_tag<To, From, Policy, 1>::type>::type;
};
// Step 1: 检查去const转换
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 1>
{
private:
  static constexpr bool is_same_underlying =
      std::is_same<remove_cv_ptr_t<To>, remove_cv_ptr_t<From>>::value;
  static constexpr bool is_const_removal =
      is_same_underlying && !std::is_same<To, From>::value &&
      !std::is_const<remove_cv_ptr_t<To>>::value &&
      std::is_const<remove_cv_ptr_t<From>>::value;

public:
  using type = typename std::conditional<
      is_const_removal && Policy::allow_const_removal, const_removal_tag,
      typename get_cast_tag<To, From, Policy, 2>::type>::type;
};
// Step 2: 检查向上转换
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 2>
{
  using type =
      std::conditional_t<is_up_cast_impl<To, From>(), up_cast_tag,
                         typename get_cast_tag<To, From, Policy, 3>::type>;
};

// Step 3: 检查向下转换（多态）
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 3>
{
  using type = typename std::conditional<
      is_down_cast_impl<To, From>() &&
          std::is_polymorphic<remove_cv_ptr_t<From>>::value,
      down_cast_polymorphic_tag,
      typename get_cast_tag<To, From, Policy, 4>::type>::type;
};

// Step 4: 检查向下转换（非多态）
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 4>
{
  using type = typename std::conditional<
      is_down_cast_impl<To, From>() &&
          !std::is_polymorphic<remove_cv_ptr_t<From>>::value &&
          Policy::allow_non_polymorphic_downcast,
      down_cast_non_polymorphic_tag,
      typename get_cast_tag<To, From, Policy, 5>::type>::type;
};

// Step 5: 检查标准指针?整数转换
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 5>
{
private:
  template <typename T>
  static constexpr bool is_standard_int()
  {
    return std::is_same<T, std::intptr_t>::value ||
           std::is_same<T, std::uintptr_t>::value;
  }

public:
  using type = std::conditional_t<
      ((std::is_pointer<From>::value && is_standard_int<To>()) ||
       (is_standard_int<From>() && std::is_pointer<To>::value)) &&
          Policy::allow_standard_pointer_integer_cast,
      standard_pointer_integer_tag,
      typename get_cast_tag<To, From, Policy, 6>::type>;
};

// Step 6: 检查通用指针?整数转换
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 6>
{
  using type = std::conditional_t<
      ((std::is_pointer<From>::value && std::is_integral<To>::value) ||
       (std::is_integral<From>::value && std::is_pointer<To>::value)) &&
          Policy::allow_reinterpret,
      generic_pointer_integer_tag,
      typename get_cast_tag<To, From, Policy, 7>::type>;
};

// Step 7: 检查标准转换
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 7>
{
  using type =
      std::conditional_t<std::is_convertible<From, To>::value,
                         standard_conversion_tag,
                         typename get_cast_tag<To, From, Policy, 8>::type>;
};

// Step 8: 检查不相关指针类型的reinterpret_cast
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 8>
{
  using type = std::conditional_t<
      std::is_pointer<To>::value && std::is_pointer<From>::value &&
          Policy::allow_reinterpret,
      reinterpret_cast_tag, typename get_cast_tag<To, From, Policy, 9>::type>;
};

// Step 9: 无法转换（递归终止）
template <typename To, typename From, typename Policy>
struct get_cast_tag<To, From, Policy, 9>
{
  using type = invalid_cast_tag;
};

// 转换实现函数
template <typename To, typename From>
To cast_impl(From from, same_type_tag)
{
  return from;
}

template <typename To, typename From>
To cast_impl(From from, up_cast_tag)
{
  return static_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, down_cast_polymorphic_tag)
{
  auto result = dynamic_cast<To>(from);
  if (!result && from) {
    throw std::bad_cast();
  }
  return result;
}

template <typename To, typename From>
To cast_impl(From from, down_cast_non_polymorphic_tag)
{
  return static_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, const_removal_tag)
{
  return const_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, standard_pointer_integer_tag)
{
  return reinterpret_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, generic_pointer_integer_tag)
{
  // 检查整数类型是否足够大
  static_assert(
      !(std::is_pointer<From>::value && std::is_integral<To>::value) ||
          (sizeof(To) >= sizeof(void*)),
      "Integer type is too small to hold a pointer");
  return reinterpret_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, standard_conversion_tag)
{
  return static_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From from, reinterpret_cast_tag)
{
  return reinterpret_cast<To>(from);
}

template <typename To, typename From>
To cast_impl(From /*unused*/, invalid_cast_tag /*unused*/)
{
  static_assert(sizeof(To) == 0,
                "auto_cast: No suitable conversion found between types");
  using T = remove_cv_ptr_t<To>;
  return T{};
}

template <typename To, typename From, typename Policy = default_policy>
struct auto_cast_impl
{
  static To cast(From from)
  {
    using tag = typename get_cast_tag<To, From, Policy>::type;
    return cast_impl<To, From>(from, tag{});
  }
};

#endif

// 用户接口 - 带策略模板参数
template <typename To, typename Policy = default_policy, typename From>
To auto_cast(From from)
{
  return auto_cast_impl<To, From, Policy>::cast(from);
}

// 便捷别名
template <typename To, typename From>
To auto_cast_safe(From from)
{
  return auto_cast_impl<To, From, default_policy>::cast(from);
}

template <typename To, typename From>
To auto_cast_unsafe(From from)
{
  return auto_cast_impl<To, From, unsafe_policy>::cast(from);
}

template <typename To, typename From>
To auto_cast_strict(From from)
{
  return auto_cast_impl<To, From, strict_policy>::cast(from);
}

#if CPP_17
// 运行时检查版本,返回std::optional
template <typename To, typename From, typename Policy = default_policy>
std::optional<To> try_auto_cast(From from) noexcept
{
  try {
    return auto_cast_impl<To, From, Policy>::cast(from);
  } catch (const std::bad_cast&) {
    return std::nullopt;
  } catch (...) {
    return std::nullopt;
  }
}
#else
// 运行时检查版本,返回空对象
template <typename To, typename From, typename Policy = default_policy>
To try_auto_cast(From from) noexcept
{
  try {
    return auto_cast_impl<To, From, Policy>::cast(from);
  } catch (const std::bad_cast&) {
    return To{};
  } catch (...) {
    return To{};
  }
}

#endif
