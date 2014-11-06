#ifndef ANDROID_LIBDLNASERVICE_DLNASERVICE_H
#define ANDROID_LIBDLNASERVICE_DLNASERVICE_H

#include "rhino_export.h"
#include "base/compiler_specific.h"
#include <string>

namespace dlna {

class RHINO_EXPORT DLNAService
{
  class DMPClient;
public:
  static  void       instantiate();

          bool       initialized();
  static  void       DMREventCallback(int eventID, const char* eventArg, std::string* out);
private:

  DLNAService();
  virtual ~DLNAService();
};

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_DLNA_SERVICE_H
