# 1.REPL
- REPL即read-execute-print-loop，一种命令行交互执行方式。
- 本仓库实现了通用的REPL封装，使用时只需要引用一个.hpp的头文件。

# 2.实现的特性
本节介绍了REPL库提供的特性。

## 2.1 光标左右移动
- 按键盘上的左右光标键时，实现了输入光标的左右移动。
- cursorMoveLeft函数实现了光标左移的功能，超过左边界时，会有响铃的声音。
- cursorMoveRight函数实现了光标右移的功能，超过右边界时，会有响铃的声音。

## 2.2 记录历史执行的命令
- 每执行一条非空命令时，都会被记录下来。

## 2.3 退格键

## 2.4【ctrl + a】

## 2.5【ctrl + u】

# 3.如何使用
直接引用repl.hpp即可。

# 4.示例
demo.cpp中展示了使用方法。
```c++
#include "repl.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  REPL repl("REPL>", "exit");
  repl.Run();
  return 0;
}
```
- 先创建一个REPL的对象实例之后，再调用Run即可。
- 如果想要实现execute的功能只需要重新创建一个类。
- 这个类它继承了REPL类，并再实现doExecute函数即可。