#include "echo_console.h"

#include "console.h"

int main(int argc, char **argv) {
  EchoConsole console("echo");
  console.Run();
  return 0;
}
