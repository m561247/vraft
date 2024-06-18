#include "echo_console.h"
#include "console.h"

int main(int argc, char **argv) {
  vraft::Console console;
  console.Run();
  return 0;
}
