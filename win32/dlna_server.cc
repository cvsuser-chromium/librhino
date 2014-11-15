#include "base/at_exit.h"

#include "dlna_service.h"
#include "AirPlayClientWin32.h"

int main(int argc, char** argv)
{
  //dlna::DLNAService::instantiate();
  base::AtExitManager at_exit;
  rhino::AirPlayClient *p = new rhino::AirPlayClient;
  //p->Initialize();


  while(1) {
    Sleep(100);
  }
  return 0;
}

