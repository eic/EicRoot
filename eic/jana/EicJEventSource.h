
#include <JANA/JApplication.h>
#include <JANA/JEventSource.h>

#ifndef _EIC_JEVENT_SOURCE_
#define _EIC_JEVENT_SOURCE_

class EicJEventSource: public JEventSource { 
 public:
  // Constructor must take string and JApplication pointer as arguments
 EicJEventSource(std::string source_name, JApplication *app): JEventSource(source_name, app) {};
  virtual ~EicJEventSource() {};

  // A description of this source type must be provided as a static member
  static std::string GetDescription(void) { return "Event source for JExample3"; }

  // See JEventSource_example3.cc for details on these;
  void Open(void);
  void GetEvent(std::shared_ptr<JEvent>);
};

#endif

