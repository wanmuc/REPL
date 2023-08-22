#include "repl.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  REPL repl("REPL>", "exit", 5);
  repl.Run();
  return 0;
}