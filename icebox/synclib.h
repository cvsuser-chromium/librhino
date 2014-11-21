#ifndef _rhino_ICEBOX_H_
#define _rhino_ICEBOX_H_

#include "base/memory/weak_ptr.h"
#include "base/memory/scoped_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/threading/non_thread_safe.h"


#include "remoting/jingle_glue/xmpp_signal_strategy.h"

namespace remoting {
class XmppSignalStrategy;
}

namespace Synclib{

class SyncHostContext;

class Synclib : public base::Thread
  , base::RefCountedThreadSafe<Synclib> {
public:
  int login();
  int getFileList();
private:
  int doLogin();
  int doGetFileList();

public:
  static Synclib* get_instance();
  int get_file_list();
//protected:
  Synclib();
  virtual ~Synclib();
protected:
  // Called just prior to starting the message loop
  virtual void Init();
  // Called to start the message loop
  virtual void Run(base::MessageLoop* message_loop);
  // Called just after the message loop ends
  virtual void CleanUp();
private:
  int init();

  // XMPP server/remoting bot configuration (initialized from the command line).
  remoting::XmppSignalStrategy::XmppServerConfig xmpp_server_config_;

  scoped_ptr<SyncHostContext> context_;
  scoped_ptr<remoting::XmppSignalStrategy> signal_strategy_;

  base::MessageLoop* message_loop_;
  base::WeakPtrFactory<Synclib> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(Synclib);
};

}
#endif
