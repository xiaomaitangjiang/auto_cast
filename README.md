# Auto Cast Library

一个现代化的C++类型转换库，提供安全、灵活的类型转换机制，支持多种转换策略和编译时检查。

## 特性

### 🛡️ 安全第一
- **编译时检查**：在编译阶段阻止不安全的类型转换
- **策略驱动**：通过策略模板控制允许的转换类型
- **多态安全**：对多态类型使用`dynamic_cast`进行运行时类型检查

### 🔧 灵活的转换策略
- **安全模式（默认）**：平衡安全性和实用性
- **不安全模式**：允许所有转换，包括`reinterpret_cast`
- **严格模式**：最严格的转换限制，禁止潜在危险操作

### 📚 丰富的转换支持
- 指针和引用类型转换
- 多态类型向上/向下转换
- 标准类型转换
- const限定符处理
- 指针与整数类型转换

### 🎯 便捷的接口
- 主模板：`auto_cast<To, Policy, From>`
- 便捷别名：`auto_cast_safe`  `auto_cast_unsafe`  `auto_cast_strict`
- 错误处理版本：`try_auto_cast`（返回`std::optional`）

## 快速开始

### 基本用法



```cpp
//只需要包含一个头文件
include "auto_cast.hpp"

// 安全向上转换

Derived* derived = new Derived();

Base* base = auto_cast<Base*>(derived);

// 多态向下转换（运行时检查）

Base* poly_base = new Derived();

if (auto result = try_auto_cast<Derived*>(poly_base)) {

(*result)->foo(); // 安全调用

}
```

### 策略示例



```cpp

// 默认安全策略

int x = 42;

uintptr_t int_ptr = auto_cast<uintptr_t>(&x);

// 不安全策略（允许reinterpret_cast）

int* ptr = auto_cast_unsafe<int*>(int_ptr);

// 严格策略（禁止危险转换）

const int y = 100;

// int& ref = auto_cast_strict<int&>(y); // 编译错误：禁止去const
```

## 转换策略对比

| 转换类型 | 安全模式 | 不安全模式 | 严格模式 |
|---------|---------|-----------|---------|
| 相同类型转换 | ✅ | ✅ | ✅ |
| const去除 | ✅ | ✅ | ❌ |
| 多态向上转换 | ✅ | ✅ | ✅ |
| 多态向下转换 | ✅（dynamic_cast） | ✅（dynamic_cast） | ✅（dynamic_cast） |
| 非多态向下转换 | ❌ | ✅ | ❌ |
| 标准转换 | ✅ | ✅ | ✅ |
| reinterpret_cast | ❌ | ✅ | ❌ |
| 指针-整数转换 | ✅ | ✅ | ❌ |

## 详细用法

### 1. 基本类型转换



```cpp

int a = 42;

double b = auto_cast<double>(a); // 标准转换

const char* str = "hello";

char* mutable_str = auto_cast<char*>(str); // 安全模式允许去const
```

### 2. 类继承转换



```cpp

class Base { virtual ~Base() = default; };

class Derived : public Base {};

// 向上转换 - 总是安全

Derived* d = new Derived();

Base* b = auto_cast<Base*>(d);

// 向下转换 - 多态类型使用dynamic_cast

Base* base_ptr = new Derived();

Derived* derived_ptr = auto_cast<Derived*>(base_ptr); // 运行时检查
```

### 3. 指针与整数转换



```cpp

int value = 42;

int* ptr = &value;

// 指针到整数（安全模式允许）

uintptr_t int_val = auto_cast<uintptr_t>(ptr);

// 整数到指针（需要不安全模式）

int* ptr2 = auto_cast_unsafe<int*>(int_val);
```

### 4. 错误处理



```cpp

Base* base = new Base(); // 不是Derived类型

// 返回std::optional，不会抛出异常

if (auto result = try_auto_cast<Derived*>(base)) {

// 转换成功

(*result)->some_method();

} else {

// 转换失败

std::cout << "转换失败\n";

}
```

## 自定义策略



```cpp

struct my_policy {

using tag = safe_cast_tag;
static constexpr bool allow_reinterpret = false;
static constexpr bool allow_const_removal = true;
static constexpr bool allow_non_polymorphic_downcast = false;
static constexpr bool allow_standard_pointer_integer_cast = true;

};

// 使用自定义策略
int result = auto_cast<int, my_policy>(some_value);

```

## 编译要求

- C++17 或更高版本
- 支持标准库`<type_traits>` `<optional>` `<cstdint>`

### 编译器支持
- GCC 7.0+
- Clang 5.0+
- MSVC 2017+

## 项目结构



project/

├── inc/

│   └── auto_cast.hpp      # 主头文件

├── examples/

│   └── main.cpp           # 使用示例

└── README.md


## 注意事项

1. **多态类型要求**：向下转换需要基类有虚函数（多态类型）
2. **引用类型**：引用转换失败会抛出`std::bad_cast`
3. **编译时错误**：违反策略的转换会在编译时报错
4. **性能考虑**：多态向下转换有运行时开销

## 许可证

本项目采用MIT许可证。详情见LICENSE文件。

## 贡献

欢迎提交Issue和Pull Request来改进这个库。

## 版本历史

- v1.0.0: 初始版本，支持基本转换策略
- 支持C++17和C++20标准
- 默认提供安全、不安全、严格三种策略模式
