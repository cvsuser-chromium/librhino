#include "base/at_exit.h"

int main(int argc, char** argv)
{
  base::AtExitManager at_exit;

   while(1) {
    Sleep(100);
  }
  return 0;
}
