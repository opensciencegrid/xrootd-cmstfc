/* 
   Implementation of a N2N class using the CMS TFC

   Rewritten, but adopted from upstream CMS sources
   Author: Brian Bockelman
   Original Author: Giulio.Eulisse@cern.ch
 */

#include <set>
#include <vector>
#include <string>

#include "XrdCmsTfc.hh"
#include "XrdSys/XrdSysError.hh"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#define BUFFSIZE 1024
#define OVECCOUNT 30

using namespace xercesc;
using namespace XrdCmsTfc;

extern "C"
{
XrdOucName2Name *XrdOucgetName2Name(XrdSysError *eDest, const char *confg,
      const char *parms, const char *lroot, const char *rroot)
{
  TrivialFileCatalog *myTFC = new TrivialFileCatalog(eDest, "/tmp/test");
  eDest->Say("Copr. 2009 University of Nebraska-Lincoln TFC plugin v 1.0"); 

  eDest->Say("Params: ");
  eDest->Say(parms);

  return myTFC;
}
}

typedef std::vector<std::string> StringVec;

StringVec &split(const std::string &s, char delim, StringVec &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


StringVec split(const std::string &s, char delim) {
    StringVec elems;
    return split(s, delim, elems);
}

inline char * _toChar(const XMLCh *toTranscode)
{
    return XMLString::transcode(toTranscode);
}

inline XMLCh*  _toDOMS(std::string temp){
    XMLCh* buff = XMLString::transcode(temp.c_str());    
    return  buff;
}

void XrdCmsTfc::TrivialFileCatalog::freeProtocolRules(ProtocolRules protRules) {
  ProtocolRules::iterator it;
  Rules::iterator it2;
  for (it = protRules.begin(); it != protRules.end(); it++) {
    Rules rules = (*it).second;
    for (it2 = rules.begin(); it2 != rules.end(); it2++) {
      if (it2->pathMatch != NULL)
        pcre_free(it2->pathMatch);
    }
  }
}

XrdCmsTfc::TrivialFileCatalog::~TrivialFileCatalog ()
{
  freeProtocolRules(m_directRules);
  freeProtocolRules(m_inverseRules);
}

int
XrdCmsTfc::TrivialFileCatalog::parseRule (DOMNode *ruleNode, 
                                          ProtocolRules &rules)
{
    if (! ruleNode)
    {
        eDest->Say("Malformed trivial catalog; passed a NULL rule");
        return XRDCMSTFC_ERR_PARSERULE;
    }
	    
    DOMElement* ruleElement = static_cast<DOMElement *>(ruleNode);	    

    if (! ruleElement) {
        eDest->Say("Xerces was unable to cast the rule node to an element.");
        return XRDCMSTFC_ERR_PARSERULE;
    }

    char * protocol 
	= _toChar (ruleElement->getAttribute (_toDOMS ("protocol")));	    
    char * destinationMatchRegexp
	= _toChar (ruleElement->getAttribute (_toDOMS ("destination-match")));

    if (strlen(destinationMatchRegexp) == 0)
	destinationMatchRegexp = ".*";

    char * pathMatchRegexp 
	= _toChar (ruleElement->getAttribute (_toDOMS ("path-match")));
    char * result 
	= _toChar (ruleElement->getAttribute (_toDOMS ("result")));
    char * chain 
	= _toChar (ruleElement->getAttribute (_toDOMS ("chain")));
    					    
    Rule rule;
    const char *error;
    int erroffset;
    rule.pathMatch = NULL;
    rule.destinationMatch = NULL;
    rule.pathMatch = pcre_compile(pathMatchRegexp, 0, &error, &erroffset, NULL);
    if (rule.pathMatch == NULL) {
      char *err = (char *)malloc(BUFFSIZE*sizeof(char));
      snprintf(err, BUFFSIZE, "PCRE compilation failed at offset %d: %s", erroffset, error);
      eDest->Say(err);
      free(err);
      return XRDCMSTFC_ERR_PARSERULE;
    }

    rule.destinationMatch = pcre_compile(destinationMatchRegexp, 0, &error, &erroffset, NULL);
    if (rule.destinationMatch == NULL) {
      char *err = (char *)malloc(BUFFSIZE*sizeof(char));
      snprintf(err, BUFFSIZE, "PCRE compilation failed at offset %d: %s", erroffset, error);
      eDest->Say(err);
      free(err);
      return XRDCMSTFC_ERR_PARSERULE;
    }
    rule.result = result;
    rule.chain = chain;
    rules[protocol].push_back (rule);
    return 0;
}

int
XrdCmsTfc::TrivialFileCatalog::parse ()
{
  	eDest->Say("Connecting to the catalog ", m_url.c_str());

	if (m_url.find ("file:") != std::string::npos) {
	    m_url = m_url.erase (0, m_url.find (":") + 1);	
	} else {
            eDest->Say("TrivialFileCatalog::connect: Malformed url for file catalog configuration");
            return XRDCMSTFC_ERR_URL;
	}

        m_filename = split(m_url, '?')[0];

        size_t ques_pos = m_url.find("?");
	if (ques_pos != std::string::npos)
	{
            m_filename = m_url.substr(0, ques_pos);
	    std::string options = m_url.substr(ques_pos+1, std::string::npos);
	    std::vector<std::string> optionTokens = split(options, '&');

	    for (StringVec::iterator option = optionTokens.begin();
		 option != optionTokens.end();
		 option++)
	    {
		StringVec argTokens = split(*option, '=') ;
		if (argTokens.size () != 2)
		{
                    eDest->Say("TrivialFileCatalog::connect: Malformed url for file catalog configuration");
                    return XRDCMSTFC_ERR_URL;
		}
		
		std::string key = argTokens[0];
		std::string value = argTokens[1];
		
		if (key == "protocol")
		{
                   StringVec prots = split(value, ',');
                   for (StringVec::iterator it = prots.begin();
                          it != prots.end(); it++)
                      m_protocols.push_back(*it);
		}
		else if (key == "destination")
		{
		    m_destination = value;
		}
	    }
	}
	
	if (m_protocols.empty()) {
            eDest->Say("TrivialFileCatalog::parse: protocol was not supplied in the contact string");
            return XRDCMSTFC_ERR_URL;
        }

	std::ifstream configFile;
	configFile.open(m_filename.c_str());
	eDest->Say("Using catalog configuration", m_filename.c_str());
	
	if (!configFile.good() || !configFile.is_open())
	{
            eDest->Say("TrivialFileCatalog::parse: Unable to open trivial file catalog", m_filename.c_str());
            return XRDCMSTFC_ERR_FILE;
	}
	
	configFile.close();
	
	XercesDOMParser* parser = new XercesDOMParser;     
	parser->setValidationScheme(XercesDOMParser::Val_Auto);
	parser->setDoNamespaces(false);
	parser->parse(m_filename.c_str());	
	DOMDocument* doc = parser->getDocument();
	assert(doc);
	
	/* trivialFileCatalog matches the following xml schema
	   FIXME: write a proper DTD
	    <storage-mapping>
	    <lfn-to-pfn protocol="direct" destination-match=".*" 
	    path-match="lfn/guid match regular expression"
	    result="/castor/cern.ch/cms/$1"/>
	    <pfn-to-lfn protocol="srm" 
	    path-match="lfn/guid match regular expression"
	    result="$1"/>
	    </storage-mapping>
	 */

	/*first of all do the lfn-to-pfn bit*/
	{
	    DOMNodeList *rules =doc->getElementsByTagName(_toDOMS("lfn-to-pfn"));
	    unsigned int ruleTagsNum  = 
		rules->getLength();
	
	    // FIXME: we should probably use a DTD for checking validity 

	    for (unsigned int i=0; i<ruleTagsNum; i++) {
		DOMNode* ruleNode =	rules->item(i);
		parseRule (ruleNode, m_directRules);
	    }
	}
	/*Then we handle the pfn-to-lfn bit*/
	{
	    DOMNodeList *rules =doc->getElementsByTagName(_toDOMS("pfn-to-lfn"));
	    unsigned int ruleTagsNum  = 
		rules->getLength();
	
	    for (unsigned int i=0; i<ruleTagsNum; i++){
		DOMNode* ruleNode =	rules->item(i);
		parseRule (ruleNode, m_inverseRules);
	    }	    
	}
}

int
XrdCmsTfc::TrivialFileCatalog::pfn2lfn(const char *pfn, char *buff, int blen) 
{
    std::string tmpLfn = "";
    std::string tmpPfn = pfn;
    
    for (std::list<std::string>::iterator protocol = m_protocols.begin();
	 protocol != m_protocols.end();
	 protocol++)
    {
	tmpLfn = applyRules(m_inverseRules, *protocol, m_destination, false, tmpPfn);
	if (!tmpLfn.empty())
	{
            strncpy(buff, tmpLfn.c_str(), blen);
	    return 0;
	}
    }
    eDest->Say("No pfn2lfn mapping for ", pfn);
    return XRDCMSTFC_ERR_NOPFN2LFN;
}

int
XrdCmsTfc::TrivialFileCatalog::lfn2pfn(const char *lfn, char *buff, int blen)
{
    std::string tmpPfn = "";
    std::string tmpLfn = lfn;
    
    for (std::list<std::string>::iterator protocol = m_protocols.begin();
         protocol != m_protocols.end();
         protocol++) 
    {    
        tmpLfn = applyRules(m_directRules, *protocol, m_destination, false, tmpLfn);
        if (!tmpLfn.empty()) {
            strncpy(buff, tmpLfn.c_str(), blen);
            return 0;
        }
    }   
    eDest->Say("No lfn2pfn mapping for ", lfn);
    return XRDCMSTFC_ERR_NOLFN2PFN;
}



std::string replace(const std::string inputString, pcre * re, std::string replacementString) {

    int ovector[OVECCOUNT], rc;
    std::string result;
    rc = pcre_exec(re, NULL, inputString.c_str(), inputString.length(), 0, 0,
        ovector, OVECCOUNT);

    if (rc <= 0) {
        return "";
    }

    int substring_end = 0, substring_begin;
    for (int i = 0; i < rc; i++) {
        substring_begin = ovector[2*i];
        result += inputString.substr(substring_end, substring_begin-substring_end) + replacementString;
        substring_end = ovector[2*i+1] + substring_begin;
    }

    return result;
}

std::string replaceWithRegexp (const int ovector[OVECCOUNT], const int rc,
		   const std::string inputString,
		   const std::string outputFormat)
{
    std::cerr << "InputString:" << inputString << std::endl;
 
    char buffer[8];
    std::string result = outputFormat;
    int substring_length;
    int substring_begin;

    for (int i = 1;
	 i < rc;
	 i++)
    {
	// If this is not true, man, we are in trouble...
	assert(i<1000000);
	sprintf(buffer, "%i", i);
	std::string variableRegexp = std::string ("[$]") + buffer;
        
        substring_begin = ovector[2*i];
        substring_length = ovector[2*i+1] - substring_begin;
        std::string matchResult = inputString.substr(substring_begin, substring_length);
	
        const char *error;
        int erroffset;
        pcre * substitutionToken = pcre_compile(variableRegexp.c_str(),
            0, &error, &erroffset, NULL);
        if (substitutionToken == NULL) {
            pcre_free(substitutionToken);
            return "";
        }
	std::cerr << "Current match: " << matchResult << std::endl;
        
	result = replace(result, substitutionToken, matchResult);

        pcre_free(substitutionToken);
    }
    return result;    
}


std::string 
XrdCmsTfc::TrivialFileCatalog::applyRules (const ProtocolRules& protocolRules,
				      const std::string & protocol,
				      const std::string & destination,
				      bool direct,
				      std::string name) const
{
    std::cerr << "Calling apply rules with protocol: " << protocol << "\n destination: " << destination << "\n " << " on name " << name << std::endl;
    
    const ProtocolRules::const_iterator rulesIterator = protocolRules.find (protocol);
    if (rulesIterator == protocolRules.end ())
	return "";
    
    const Rules &rules=(*(rulesIterator)).second;
    
    /* Look up for a matching rule*/
    for (Rules::const_iterator i = rules.begin ();
	 i != rules.end ();
	 i++)
    {
        int ovector[OVECCOUNT];
        int rc=0;
        pcre_exec(i->destinationMatch, NULL, destination.c_str(), destination.length(), 0, 0, ovector, OVECCOUNT);
	if (rc < 0)
	    continue;
	
        pcre_exec(i->pathMatch, NULL, name.c_str(), name.length(), 0, 0, ovector, OVECCOUNT);
	if (rc < 0)
	    continue;
	
	std::cerr << "Rule matched! " << std::endl;	
	
	std::string chain = i->chain;
	if ((direct==true) && (chain != ""))
	{
	    name = 
		applyRules (protocolRules, chain, destination, direct, name);		
	}
	    
        pcre_exec(i->pathMatch, NULL, name.c_str(), name.length(), 0, 0, ovector,
            OVECCOUNT);

        if (rc > 0) {
            name = replaceWithRegexp(ovector, rc, name, i->result);
        }
	
	if ((direct == false) && (chain !=""))
	{	
	    name = 
		applyRules (protocolRules, chain, destination, direct, name);		
	}
	
	return name;
    }
    return "";
}

