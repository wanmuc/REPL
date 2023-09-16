#pragma once

#include <signal.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <string>

class REPL {
  enum InputStatus {
    INIT = 1,  // 初始化
    ESC = 2,  // ESC
    DOUBLE_ESC = 3,  // 双重ESC
    CURSOR_MOVE_CHAR = 4,  // 按字符移动光标
    CURSOR_MOVE_WORD = 5,  // 按单词移动光标
  };

 public:
  REPL(const std::string& prefix, const std::string& continue_prefix, const std::string& exit_cmd,
       size_t cmd_max_reserved_cnt) {
    prefix_ = prefix;
    exit_cmd_ = exit_cmd;
    continue_prefix_ = continue_prefix;
    is_complete_cmd_ = true;
    cmd_max_reserved_cnt_ = cmd_max_reserved_cnt;
    tcgetattr(STDIN_FILENO, &term_old_attr_);  // 获取终端属性
    term_new_attr_ = term_old_attr_;
    term_new_attr_.c_lflag &= ~(ICANON | ECHO);  // 关闭标准输入模式和回显
    tcsetattr(STDIN_FILENO, TCSANOW, &term_new_attr_);  // 设置终端属性
  }
  void Run() {
    std::string output;
    std::string cmd_line;
    while (not isExit()) {
      printPrefix();
      read(cmd_line);
      execute(cmd_line, output);
      print(output);
    }
  }

 private:
  bool isExit() {
    if (exit_cmd_ == last_execute_cmd_) {
      tcsetattr(STDIN_FILENO, TCSANOW, &term_old_attr_);  // 恢复终端属性
      return true;
    }
    return false;
  }
  void printPrefix() {
    if (is_complete_cmd_) {
      printf("%s ", prefix_.c_str());
    } else {
      printf("%s ", continue_prefix_.c_str());
    }
  }
  void read(std::string& cmd_line) {
    char ch;
    cmd_line = "";
    constexpr char kEsc = 27;  // Esc键
    constexpr char kCtrlA = 1;  // 【Ctrl + a】-> 输入光标移动到行首
    constexpr char kCtrlE = 5;  // 【Ctrl + e】-> 输入光标移动到行尾
    constexpr char kCtrlU = 21;  // 【Ctrl + u】-> 清空当前命令行的输入
    constexpr char kBackspace = 127;  // 回退键
    int cursor_pos = 0;  // 光标位置，初始化为0
    int cur_history_cmd_pos = history_cmds_.size();  // 当前历史命令的位置
    int input_status = INIT;
    while (true) {
      ch = getchar();
      if (ch == '\n') {
        printf("%c", ch);
        return;
      }
      if (input_status == INIT) {
        if (ch == kBackspace) {
          backSpace(cursor_pos, cmd_line);
        } else if (ch == kCtrlA) {
          cursorMoveHead(cursor_pos);
        } else if (ch == kCtrlE) {
          cursorMoveEnd(cursor_pos, cmd_line.size());
        } else if (ch == kCtrlU) {
          clearPrefixCmdLine(cursor_pos, cmd_line);
        } else if (isprint(ch)) {
          printChar(ch, cursor_pos, cmd_line);
        } else if (ch == kEsc) {
          input_status = ESC;
        }
        continue;
      } else if (input_status == ESC) {
        if (ch == kEsc) {
          input_status = DOUBLE_ESC;
        } else if (ch == '[') {
          input_status = CURSOR_MOVE_CHAR;
        } else {
          assert(0);
        }
        continue;
      } else if (input_status == DOUBLE_ESC) {
        if (ch == '[') {
          input_status = CURSOR_MOVE_WORD;
        } else {
          assert(0);
        }
        continue;
      } else if (input_status == CURSOR_MOVE_CHAR) {
        if (ch == 'A') {  // 上移光标-上一条历史命令
          showPreCmd(cur_history_cmd_pos, cursor_pos, cmd_line);
        } else if (ch == 'B') {  // 下移光标-下一条历史命令
          showNextCmd(cur_history_cmd_pos, cursor_pos, cmd_line);
        } else if (ch == 'C') {  // 右移光标
          cursorMoveRight(cursor_pos, cmd_line);
        } else if (ch == 'D') {  // 左移光标
          cursorMoveLeft(cursor_pos);
        } else {
          assert(0);
        }
        input_status = INIT;
        continue;
      } else if (input_status == CURSOR_MOVE_WORD) {
        if (ch == 'C') {
          cursorMoveRightOneWord(cursor_pos, cmd_line);
        } else if (ch == 'D') {
          cursorMoveLeftOneWord(cursor_pos, cmd_line);
        } else {
          assert(0);
        }
        input_status = INIT;
      }
    }
  }
  void print(std::string& output) {
    if (output != "") {
      printf("%s\n", output.c_str());
    }
  }
  void printChar(char ch, int& cursor_pos, std::string& cmd_line) {
    if (cursor_pos == cmd_line.size()) {  // 光标在输入的尾部，则把字符插入尾部
      printf("%c", ch);
      cursor_pos++;
      cmd_line += ch;
      return;
    }
    // 执行到这里表示光标在输入的中间，除了输出当前字符之外，还需要把后面的字符往后移动一格
    std::string tail = cmd_line.substr(cursor_pos);
    cmd_line.insert(cursor_pos, 1, ch);
    cursor_pos++;
    printf("%c%s", ch, tail.c_str());
    for (size_t i = 0; i < tail.size(); i++) {  // 光标需要退回到插入的位置
      printf("\033[1D");
    }
  }
  void showPreCmd(int& cur_history_cmd_pos, int& cursor_pos, std::string& cmd_line) {
    if (history_cmds_.size() > 0 && cur_history_cmd_pos > 0) {  // 有历史命令才处理
      clearCmdLine(cursor_pos, cmd_line);
      cur_history_cmd_pos--;
      cmd_line = history_cmds_[cur_history_cmd_pos];
      cursor_pos = cmd_line.size();
      printf("%s", cmd_line.c_str());  // 打印被选择的历史命令行
    } else {
      printf("\a");
    }
  }

  void showNextCmd(int& cur_history_cmd_pos, int& cursor_pos, std::string& cmd_line) {
    if (history_cmds_.size() > 0 && cur_history_cmd_pos < history_cmds_.size() - 1) {  // 有历史命令才处理
      clearCmdLine(cursor_pos, cmd_line);
      cur_history_cmd_pos++;
      cmd_line = history_cmds_[cur_history_cmd_pos];
      cursor_pos = cmd_line.size();
      printf("%s", cmd_line.c_str());  // 打印被选择的历史命令行
    } else {
      printf("\a");
    }
  }
  void backSpace(int& cursor_pos, std::string& cmd_line) {
    if (cursor_pos <= 0) {
      printf("\a");
      return;  // 光标已经到最左边了
    }
    if (cursor_pos == cmd_line.size()) {  // 光标在输入的最后
      cursor_pos--;
      printf("\b \b");  // 删除光标前面的字符（退一格、输出空白符、退一格）
      cmd_line = cmd_line.substr(0, cmd_line.size() - 1);
      return;
    }
    //  执行到这里，说明是在光标在输入的中间，需要删除光标前面的一个字符，并把后面的字段都向前移动一格
    std::string tail = cmd_line.substr(cursor_pos);
    cursor_pos--;
    printf("\b");  // 退一格
    for (size_t i = cursor_pos; i < cmd_line.size(); i++) {  // 抹掉后面的输出
      printf(" ");
    }
    for (size_t i = cursor_pos; i < cmd_line.size(); i++) {  // 光标回退
      printf("\b");
    }
    printf("%s", tail.c_str());  // 这个时候打印，实现"后面的字符都向前移动一格"
    for (size_t i = 0; i < tail.size(); i++) {
      printf("\b");  // 光标再回退
    }
    // 删除cmd_line中的字符
    cmd_line.erase(cursor_pos, 1);
  }
  void clearCmdLine(int& cursor_pos, std::string& cmd_line) {
    for (size_t i = cursor_pos; i < cmd_line.size(); i++) {  // 清空光标后面的内容
      printf(" ");
    }
    for (size_t i = cursor_pos; i < cmd_line.size(); i++) {  // 光标回退
      printf("\b");
    }
    while (cursor_pos > 0) {  // 清空终端当前行打印的内容
      // 通过光标回退一格，然后打印空白符，最后再回退一格的方来实现命令行输入中最后一个字符的清除
      printf("\b \b");
      cursor_pos--;
    }
    cmd_line = "";  // 清空命令行
  }
  void clearPrefixCmdLine(int& cursor_pos, std::string& cmd_line) {
    for (int i = 0; i < cursor_pos; i++) {  // 光标移动到输入起点
      printf("\b");
    }
    for (size_t i = 0; i < cmd_line.size(); i++) {  // 清空输入的内容
      printf(" ");
    }
    for (size_t i = 0; i < cmd_line.size(); i++) {  // 光标退回输入起点
      printf("\b");
    }
    cmd_line = cmd_line.substr(cursor_pos);
    printf("%s", cmd_line.c_str());
    for (size_t i = 0; i < cmd_line.size(); i++) {  // 光标退回输入起点
      printf("\b");
    }
    cursor_pos = 0;
  }
  void cursorMoveHead(int& cursor_pos) {
    for (int i = 0; i < cursor_pos; i++) {
      printf("\033[1D");
    }
    cursor_pos = 0;
  }
  void cursorMoveEnd(int& cursor_pos, int cmd_line_len) {
    while (cursor_pos < cmd_line_len) {
      printf("\033[1C");  // 光标右移一格的组合
      cursor_pos++;
    }
  }
  void cursorMoveLeft(int& cursor_pos) {
    if (cursor_pos > 0) {
      printf("\033[1D");  // 光标左移一格的组合
      cursor_pos--;
    } else {
      printf("\a");
    }
  }
  void cursorMoveRight(int& cursor_pos, const std::string& cmd_line) {
    if (cursor_pos < cmd_line.size()) {
      printf("\033[1C");  // 光标右移一格的组合
      cursor_pos++;
    } else {
      printf("\a");
    }
  }
  void cursorMoveLeftOneWord(int& cursor_pos, const std::string& cmd_line) {
    // 一直往左退一格，只要光标前面的字符是空白符
    while (cursor_pos - 1 >= 0 && isblank(cmd_line[cursor_pos - 1])) {
      printf("\033[1D");  // 光标左移一格的组合
      cursor_pos--;
    }
    // 一直往左退一格，只要光标前面的字符是非空白符
    while (cursor_pos - 1 >= 0 && !isblank(cmd_line[cursor_pos - 1])) {
      printf("\033[1D");  // 光标左移一格的组合
      cursor_pos--;
    }
  }
  void cursorMoveRightOneWord(int& cursor_pos, const std::string& cmd_line) {
    // 一直往右退一格，只要光标后面的字符是空白符
    while (cursor_pos + 1 < cmd_line.size() && isblank(cmd_line[cursor_pos + 1])) {
      printf("\033[1C");  // 光标右移一格的组合
      cursor_pos++;
    }
    // 一直往右退一格，只要光标后面的字符是非空白符
    while (cursor_pos + 1 < cmd_line.size() && !isblank(cmd_line[cursor_pos + 1])) {
      printf("\033[1C");  // 光标右移一格的组合
      cursor_pos++;
    }
    // 光标需要在单词的后面，故这里需要再退一格
    if (cursor_pos + 1 <= cmd_line.size()) {
      printf("\033[1C");  // 光标右移一格的组合
      cursor_pos++;
    }
  }
  void trim(std::string& str) {
    if (str.empty()) return;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
  }
  void execute(std::string& cmd_line, std::string& output) {
    output = "";
    trim(cmd_line);
    is_complete_cmd_ = true;
    if (cmd_line != "") {
      last_execute_cmd_ = cmd_line;
      history_cmds_.push_back(last_execute_cmd_);
      // 只保留最近cmd_max_reserved_cnt_条执行的命令
      if (history_cmds_.size() > cmd_max_reserved_cnt_) {
        history_cmds_.erase(history_cmds_.begin());
      }
      is_complete_cmd_ = handlerCmdLine(cmd_line, output);
    }
  }
  virtual bool handlerCmdLine(std::string& cmd_line, std::string& output) {
    output = "handler[" + cmd_line + "]";
    return true;
  }

 private:
  std::string prefix_;  // 在命令行终端中等待用户输入而输出的提示前缀
  std::string continue_prefix_;  // 用户输入的命令还不完整时，需要等待用户继续输入时输出的提示前缀
  std::string exit_cmd_;  // 退出命令
  std::string last_execute_cmd_;  // 最后一次执行的命令
  bool is_complete_cmd_;  // 用户当前输入的命令是否完成
  std::vector<std::string> history_cmds_;  // 保留历史命令的列表
  size_t cmd_max_reserved_cnt_;  // 最多保留最近多少条历史命令
  struct termios term_old_attr_;  // 旧的终端属性
  struct termios term_new_attr_;  // 新的终端属性
};