
#ifndef _ICEBOX_H_
#define _ICEBOX_H_

namespace synclib{
class synclib {
public:
  synclib* get_instance();
  ~synclib();

  int get_file_list();
private:
  synclib();
  int init();
};

}
#endif
