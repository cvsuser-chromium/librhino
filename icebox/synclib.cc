#include "base/bind.h"
#include "base/logging.h"
#include "base/lazy_instance.h"

#include "synclib.h"

namespace Synclib{

base::LazyInstance<Synclib> gSynclib = LAZY_INSTANCE_INITIALIZER;

Synclib* Synclib::get_instance()
{
  return gSynclib.Pointer();
}

Synclib::Synclib()
  : base::Thread("synclib")
  , message_loop_(0)
  , weak_factory_(this)
{
  init();
}
int Synclib::init()
{
  Options ops(base::MessageLoop::TYPE_IO, 0);
  StartWithOptions(ops);
  return 0;
}
Synclib::~Synclib()
{
}
void Synclib::Init()
{
  message_loop_ = base::MessageLoop::current();
}
void Synclib::Run(base::MessageLoop* message_loop)
{
  Thread::Run(message_loop);
}
void Synclib::CleanUp()
{
}


int Synclib::login()
{
  if (message_loop_)
    message_loop_->PostNonNestableTask(FROM_HERE,
      base::Bind(base::IgnoreResult(&Synclib::doLogin), weak_factory_.GetWeakPtr())); 
  return 0;
}
int Synclib::getFileList()
{
  return 0;
}

int Synclib::doLogin()
{
  LOG(INFO) << "run here.";
  return 0;
}
int Synclib::doGetFileList()
{
  return 0;
}

}

