#include "repl.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  REPL repl("REPL>", "exit");
  repl.Run();
  return 0;
}