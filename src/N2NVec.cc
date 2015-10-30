
#include "XrdOuc/XrdOucName2Name.hh"

#include "XrdCmsTfc.hh"
#include "N2NVec.hh"

using namespace XrdCmsTfc;

std::vector<std::string *> *
N2NVec::n2nVec(const char *lfn)
{
  std::vector<std::string *> *result = new std::vector<std::string *>();
  std::string *pfn = new std::string();
  result->push_back(pfn);
  m_tfc.lfn2pfn(lfn, *pfn);
  return result;
}


