#ifndef _rhino_ICEBOX_H_
#define _rhino_ICEBOX_H_

#include "base/memory/weak_ptr.h"
//#include "base/synchronization/lock.h"
#include "base/threading/non_thread_safe.h"

namespace Synclib{
class Synclib : public base::RefCountedThreadSafe<Synclib> {
public:  
  static Synclib* get_instance();
  int get_file_list();
//protected:
  Synclib();    
  virtual ~Synclib();
private:
  int init();

  base::WeakPtrFactory<Synclib> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(Synclib);
};

}
#endif
