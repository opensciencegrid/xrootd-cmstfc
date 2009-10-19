#ifndef TRIVIALFILECATALOG_H
#define TRIVIALFILECATALOG_H

#include "XrdOuc/XrdOucName2Name.hh"
#include "XrdSys/XrdSysError.hh"

#include <list>
#include <utility>
#include <xercesc/dom/DOM.hpp>

namespace XrdCmsTfc
{
/**
 *        @class TrivialFileCatalog 
 *          	This class is the concrete implementation of the 
 *          	CMS Trivial File Catalog as implemented by Giulio Eulisse
 *              The adoptation was then done for Xrd to be a Name2Name module.
 *       @Author: Giulio Eulisse
 *       @Author: Brian Bockelman
 */

#define XRDCMSTFC_OK 0
#define XRDCMSTFC_ERR_PARSERULE 1

class TrivialFileCatalog : XrdOucName2Name
{
public:

    TrivialFileCatalog (XrdSysError *lp, const char * tfc_file) : XrdOucName2Name()
       {m_url = tfc_file; eDest = lp;}

    virtual ~TrivialFileCatalog ();

    int pfn2lfn(const char *pfn, char *buff, int blen);
  
    int lfn2pfn(const char *lfn, char *buff, int blen);

    int lfn2rfn(const char *lfn, char *buff, int blen);
 
private:
    mutable bool 	m_connectionStatus;
    
    typedef struct {
	pcre pathMatch;
	pcre destinationMatch;	
	std::string result;
	std::string chain;
    } Rule;

    typedef std::list <Rule> Rules;
    typedef std::map <std::string, Rules> ProtocolRules;

    void parseRule (xercesc::DOMNode *ruleNode, 
		    ProtocolRules &rules);
    
    std::string applyRules (const ProtocolRules& protocolRules,
			    const std::string & protocol,
			    const std::string & destination,
			    bool direct,
			    std::string name) const;
    

            
    /** Direct rules are used to do the mapping from LFN to PFN.*/
    ProtocolRules	 	m_directRules;
    /** Inverse rules are used to do the mapping from PFN to LFN*/
    ProtocolRules		m_inverseRules;
    
    std::string 		m_fileType;
    std::string			m_filename;
    std::list <std::string>	m_protocols;
    std::string			m_destination;    
    const char *                m_url;

    static XrdSysError *eDest;

};    
    

}

extern "C"
{
XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs);
}

#endif

