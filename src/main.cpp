#include <iostream>
#include "../inc/auto_cast.hpp"

// 使用示例
class Base
{
public:
  virtual ~Base() = default;
  virtual void foo() { std::cout << "Base::foo()\n"; }
};

class Derived : public Base
{
public:
  void foo() override { std::cout << "Derived::foo()\n"; }
};

class NonPolymorphicBase
{
};

class NonPolymorphicDerived : public NonPolymorphicBase
{
};

// 自定义策略
struct my_policy
{
  using tag = safe_cast_tag;
  static constexpr bool allow_reinterpret = false;               // 禁止reinterpret
  static constexpr bool allow_const_removal = true;              // 允许去const
  static constexpr bool allow_non_polymorphic_downcast = false;  // 禁止非多态向下转换
};

void demonstrate_different_policies()
{
  std::cout << "=== 演示不同策略的auto_cast ===\n\n";

  // 1. 默认策略（安全模式）
  std::cout << "1. 默认策略（安全模式）:\n";

  int x = 42;
  int* ptr = &x;

  // 安全模式允许的转换
  uintptr_t int_ptr1 = auto_cast<uintptr_t>(ptr);  // 标准转换
  std::cout << "   标准转换: " << int_ptr1 << "\n";

  // 安全模式禁止reinterpret_cast
  // 以下代码在编译时会报错：
  // int* ptr2 = auto_cast<int*>(int_ptr1);  // 错误：不允许reinterpret_cast

  // 2. 不安全模式
  std::cout << "\n2. 不安全模式:\n";

  // 明确使用不安全模式
  int* ptr3 = auto_cast<int*, unsafe_policy>(int_ptr1);
  std::cout << "   reinterpret_cast允许: ptr3 = " << ptr3 << "\n";

  // 或者使用便捷别名
  int* ptr4 = auto_cast_unsafe<int*>(int_ptr1);
  std::cout << "   便捷别名: ptr4 = " << ptr4 << "\n";

  // 3. 严格模式
  std::cout << "\n3. 严格模式:\n";

  int y = 100;
  // 严格模式允许的转换
  int int_y = auto_cast<int>(y);  // 允许，没有去const
  std::cout << "   标准转换: " << int_y << "\n";

  // 严格模式禁止的转换
  // 以下代码在编译时会报错：
  // int& ref_y = auto_cast_strict<int&>(y);  // 错误：不允许去const

  // 4. 非多态向下转换
  std::cout << "\n4. 非多态向下转换:\n";

  NonPolymorphicDerived npd;
  NonPolymorphicBase* npb = &npd;

  // 默认策略允许
  NonPolymorphicDerived* npd2 = auto_cast<NonPolymorphicDerived*>(npb);
  std::cout << "   默认策略允许非多态向下转换\n";

  // 严格模式禁止
  // 以下代码在编译时会报错：
  // NonPolymorphicDerived* npd3 =
  //     auto_cast_strict<NonPolymorphicDerived*>(npb);  //
  //     错误：不允许非多态向下转换

  // 5. 运行时安全版本
  std::cout << "\n5. 运行时安全版本:\n";

  Base* base = new Derived();

  // 使用try_auto_cast，返回optional
  if (auto derived = try_auto_cast<Derived*>(base)) {
    std::cout << "   转换成功\n";
    (*derived)->foo();
  }

  // 错误的向下转换
  Base* base2 = new Base();
  if (auto derived2 = try_auto_cast<Derived*>(base2)) {
    std::cout << "   转换成功（不应该打印）\n";
  }
  else {
    std::cout << "   转换失败，返回nullopt\n";
  }

  // 6. 不同策略的组合
  std::cout << "\n6. 策略组合示例:\n";

  

  const int z = 200;
  int ref_z = auto_cast<int, my_policy>(z);
  std::cout << "   自定义策略：允许去const，禁止reinterpret和非多态向下转换\n";

  delete base;
  delete base2;
}

int main()
{
  demonstrate_different_policies();
  return 0;
}