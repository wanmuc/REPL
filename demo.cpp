#include "repl.hpp"

using namespace std;

class MyREPL : public REPL {
 public:
  MyREPL(const std::string& prefix, const std::string& continue_prefix, const std::string& exit_cmd,
         size_t cmd_max_reserved_cnt)
      : REPL(prefix, continue_prefix, exit_cmd, cmd_max_reserved_cnt) {}

 private:
  bool handlerCmdLine(std::string& cmd_line, std::string& output) override {
    output = "handler[" + cmd_line + "]";
    if (cmd_line[cmd_line.size() - 1] == ';') {
      return true;
    } else {
      return false;
    }
  }
};

int main(int argc, char* argv[]) {
  MyREPL repl("REPL>", "....>", "exit", 5);
  repl.Run();
  return 0;
}