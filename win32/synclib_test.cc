#include "base/at_exit.h"

#include "rhino/icebox/synclib.h"


int main(int argc, char** argv)
{
  base::AtExitManager at_exit;
  Synclib::Synclib* psync = Synclib::Synclib::get_instance();


   while(1) {
    Sleep(100);
  }
  return 0;
}

