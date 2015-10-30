#ifndef __N2NVEC_H_
#define __N2NVEC_H_

#include <XrdOuc/XrdOucName2Name.hh>

namespace XrdCmsTfc {

class TrivialFileCatalog;

class N2NVec : public XrdOucName2NameVec {

public:

  virtual std::vector<std::string *> *n2nVec(const char *lfn);
  virtual ~N2NVec() {}
  N2NVec(TrivialFileCatalog &tfc) : m_tfc(tfc) {}

private:

  TrivialFileCatalog &m_tfc;

};

}

#endif  // __N2NVEC_H_
