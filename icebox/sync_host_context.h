#ifndef RHINO_ICEBOX_SYNC_HOST_CONTEXT_H_
#define RHINO_ICEBOX_SYNC_HOST_CONTEXT_H_

#include "base/gtest_prod_util.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace net {
class URLRequestContextGetter;
}  // namespace net

namespace remoting {
class AutoThreadTaskRunner;
}
namespace Synclib {

// A class that manages threads and running context for the chromoting host
// process.  This class is virtual only for testing purposes (see below).
class SyncHostContext {
 public:
  ~SyncHostContext();

  // Create threads and URLRequestContextGetter for use by a host.
  // During shutdown the caller should tear-down the ChromotingHostContext and
  // then continue to run until |ui_task_runner| is no longer referenced.
  // NULL is returned if any threads fail to start.
  static scoped_ptr<SyncHostContext> Create(
      scoped_refptr<remoting::AutoThreadTaskRunner> ui_task_runner);

  // Task runner for the thread that is used for blocking file
  // IO. This thread is used by the URLRequestContext to read proxy
  // configuration and by NatConfig to read policy configs.
  scoped_refptr<remoting::AutoThreadTaskRunner> file_task_runner();

  // Task runner for the thread used for network IO. This thread runs
  // a libjingle message loop, and is the only thread on which
  // libjingle code may be run.
  scoped_refptr<remoting::AutoThreadTaskRunner> network_task_runner();

  // Task runner for the thread that is used for the UI. In the NPAPI
  // plugin this corresponds to the main plugin thread.
  scoped_refptr<remoting::AutoThreadTaskRunner> ui_task_runner();

  scoped_refptr<net::URLRequestContextGetter> url_request_context_getter();

 private:
  SyncHostContext(remoting::AutoThreadTaskRunner* ui_task_runner);

  // Thread for I/O operations.
  scoped_refptr<remoting::AutoThreadTaskRunner> file_task_runner_;


  // Thread for network operations.
  scoped_refptr<remoting::AutoThreadTaskRunner> network_task_runner_;

  // Caller-supplied UI thread. This is usually the application main thread.
  scoped_refptr<remoting::AutoThreadTaskRunner> ui_task_runner_;

  // Serves URLRequestContexts that use the network and UI task runners.
  scoped_refptr<net::URLRequestContextGetter> url_request_context_getter_;

  DISALLOW_COPY_AND_ASSIGN(SyncHostContext);
};

}  // namespace synclib

#endif  // RHINO_ICEBOXSYNC_HOST_CONTEXT_H_
