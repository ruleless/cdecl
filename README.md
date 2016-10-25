# C声明语句分析小工具

## 简介

本工具参考《C专家编程》此书，可在此书中找到本工具的原理！

要理解复杂的C语言声明语句，先要理解声明的优先级规则：

  1. 声明从它的名字(变量名)开始读取，然后按照优先级顺序依次读取

  2. 优先级从高到低依次是：

     1. 声明中被括号括起来的部分优先级最高
	 2. 其次是后缀操作符：小括号()表示这是一个函数，而中括号[]表示这是一个数组

  3. 如果 const 和(或) volatile 关键字的后面紧跟类型说明符(如 int, long 等)，
     那么它作用于类型说明符。
	 在其他情况下，const 和(或) volatile 作用于它左边紧邻的指针星号

## 编译

使用命令 `make` 直接编译即可，编译成功后，会在 `src` 目录下生成可执行文件 `cdecl`。

## 示例

  * `input:` int a; `output:` declare a as (int)
  * `input:` char *a; `output:` declare a as (pointer to char)
  * `input:` const char *const p; `output:` declare p as (const pointer to const char)
  * `input:` volatile const signed int *const volatile p; `output:` declare p as (const volatile pointer to volatile const signed int)
  * `input:` char *const *(*p)(); `output:` declare p as (pointer to function return pointer to const pointer to char)
  * `input:` char *(*c[10])(int **p); `output:` declare c as (array with pointer to function return pointer to char

## Bugs

  1. 不支持对含有结构、联合、枚举等数据结构的声明语句的分析
  2. 分析工具将无视数组括号内的任何内容，例如：`int a[100aa]` 将被分析为 `array with int`，而不会提示语法错误
  3. 分析工具将无视函数括号内的任何内容，例如：`void f(in a, out bb)` 将被分析为 `function return void`，而不会提示语法错误
