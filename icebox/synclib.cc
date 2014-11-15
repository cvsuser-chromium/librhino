#include "base/lazy_instance.h"

#include "synclib.h"

namespace Synclib{

base::LazyInstance<Synclib> gSynclib = LAZY_INSTANCE_INITIALIZER;
Synclib* Synclib::get_instance()
{
  return gSynclib.Pointer();
}

Synclib::Synclib()
  : weak_factory_(this)
{
  init();
}
Synclib::~Synclib()
{
}

int Synclib::init()
{
  return 0;
}
int Synclib::get_file_list()
{
  return 0;
}
}

