#include "synclib.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "rhino/icebox/sync_host_context.h"
#include "net/socket/client_socket_factory.h"
#include "net/url_request/url_request_context_getter.h"
#include "remoting/base/auto_thread_task_runner.h"

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
  // Enable support for SSL server sockets, which must be done while still
  // single-threaded.
  //net::EnableSSLServerSockets();
  {
    Options ops(base::MessageLoop::TYPE_IO, 0);
    StartWithOptions(ops);
  }
  return 0;
}
Synclib::~Synclib()
{
}
void Synclib::Init()
{
  base::MessageLoop* message_loop = base::MessageLoop::current();

  context_ =
    SyncHostContext::Create(new remoting::AutoThreadTaskRunner(
      message_loop->message_loop_proxy(), base::MessageLoop::QuitClosure()));

  xmpp_server_config_.host = "192.168.202.248";

  xmpp_server_config_.port = 0;
  xmpp_server_config_.use_tls = false;
  xmpp_server_config_.username = "guohongqi@hybroad.com/win32";
  xmpp_server_config_.auth_token = "";
  xmpp_server_config_.auth_service = "";

  signal_strategy_.reset(
      new remoting::XmppSignalStrategy(net::ClientSocketFactory::GetDefaultFactory(),
                             context_->url_request_context_getter(),
                             xmpp_server_config_));

  message_loop_ = message_loop;
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
  if (message_loop_) {
    message_loop_->PostNonNestableTask(FROM_HERE,
      base::Bind(base::IgnoreResult(&Synclib::doLogin), weak_factory_.GetWeakPtr()));
  } else {
    LOG(WARNING) << "Message loop not started yet!, please try again later.";
  }
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

