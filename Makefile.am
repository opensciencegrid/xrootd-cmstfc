#######################################################################
## Makefile.am for xrootd
##
##
## Initial version: 1.8.2005
##
## Version info: $Id: Makefile.am,v 1.2 2007/06/07 10:45:05 dfeich Exp $
## Checked in by $Author: dfeich $
#######################################################################

lib_LTLIBRARIES = libXrdCmsTfc.la

xrootdsfsincdir = $(includedir)/xrootd/XrdCmsTfc

xrootdsfsinc_HEADERS = XrdCmsTfc.hh

INCLUDES = -I$(top_srcdir)/src

libXrdCmsTfc_la_SOURCES = \
         XrdCmsTfc.cc XrdCmsTfc.hh

libXrdCmsTfc_la_LDFLAGS = -lxerces-c -lpcre

