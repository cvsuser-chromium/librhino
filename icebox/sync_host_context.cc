#include "rhino/icebox/sync_host_context.h"

#include <string>

#include "base/bind.h"
#include "remoting/base/auto_thread.h"
#include "remoting/base/url_request_context.h"

namespace Synclib {

SyncHostContext::SyncHostContext(
    remoting::AutoThreadTaskRunner* ui_task_runner)
    : ui_task_runner_(ui_task_runner) {

  file_task_runner_ = remoting::AutoThread::CreateWithType(
      "SyncFileThread", ui_task_runner_, base::MessageLoop::TYPE_IO);
  network_task_runner_ = remoting::AutoThread::CreateWithType(
      "SyncNetworkThread", ui_task_runner_, base::MessageLoop::TYPE_IO);

  url_request_context_getter_ = new remoting::URLRequestContextGetter(
      network_task_runner_);
}

SyncHostContext::~SyncHostContext() {
}

scoped_ptr<SyncHostContext> SyncHostContext::Create(
    scoped_refptr<remoting::AutoThreadTaskRunner> ui_task_runner) {
  DCHECK(ui_task_runner->BelongsToCurrentThread());

  scoped_ptr<SyncHostContext> context(
      new SyncHostContext(ui_task_runner.get()));
  if (!context->file_task_runner_.get() ||
      !context->network_task_runner_.get() ||
      !context->url_request_context_getter_.get()) {
    context.reset();
  }

  return context.Pass();
}

scoped_refptr<remoting::AutoThreadTaskRunner>
SyncHostContext::file_task_runner() {
  return file_task_runner_;
}

scoped_refptr<remoting::AutoThreadTaskRunner>
SyncHostContext::network_task_runner() {
  return network_task_runner_;
}

scoped_refptr<remoting::AutoThreadTaskRunner>
SyncHostContext::ui_task_runner() {
  return ui_task_runner_;
}

scoped_refptr<net::URLRequestContextGetter>
SyncHostContext::url_request_context_getter() {
  return url_request_context_getter_;
}

}  // namespace remoting
