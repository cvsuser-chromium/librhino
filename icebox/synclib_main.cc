#include "base/lazy_instance.h"

#include "synclib.h"

namespace synclib{

base::LazyInstance<synclib> gSynclib = LAZY_INSTANCE_INITIALIZER;
synclib* synclib::get_instance()
{
  return gSynclib.Pointer();
}

synclib::synclib()
{
  init();
}
synclib::~synclib()
{
}

int synclib::init()
{

}
int synclib::get_file_list()
{
  return 0;
}
}
