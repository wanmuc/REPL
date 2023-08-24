# 1.REPL
- REPL即read-execute-print-loop，一种命令行交互执行方式。
- 本仓库实现了通用的REPL封装，使用时只需要引用一个.hpp的头文件。
- REPL执行流程如下图所示。

![img.png](https://github.com/wanmuc/REPL/blob/main/img.png#pic_center)

# 2.实现的特性
本节介绍了REPL库提供的特性。

## 2.1 光标左右移动
- 按键盘上的左右光标键时，实现了输入光标的左右移动。
- cursorMoveLeft函数实现了光标左移的功能，超过左边界时，会有响铃的声音。
- cursorMoveRight函数实现了光标右移的功能，超过右边界时，会有响铃的声音。

## 2.2 记录历史执行的命令
- 只有非空命令，都会被记录下来。
- 可以设置最多保留多少条最近执行的命令。
- 通过上下光标键可以翻阅历史执行的命令。

## 2.3 退格键
- 每按一次退格键就是删除光标前面的一个字符。
- 如果没有可以删除的字符，会有响铃的声音。

## 2.4【ctrl + a】
- 把光标移动到起始的输入处。

## 2.5【ctrl + u】
- 从当前光标为界，删除光标左侧所有的字符。

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

# 5.微信公众号
欢迎关注微信公众号「Linux后端开发工程实践」，第一时间获取最新文章！扫码即可订阅。
![img.png](https://github.com/wanmuc/REPL/blob/main/mp_account.png#pic_center=660*180)