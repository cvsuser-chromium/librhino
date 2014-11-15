#ifndef _rhino_ICEBOX_H_
#define _rhino_ICEBOX_H_

#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "base/threading/non_thread_safe.h"

namespace Synclib{
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


  base::MessageLoop* message_loop_;
  base::WeakPtrFactory<Synclib> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(Synclib);
};

}
#endif
