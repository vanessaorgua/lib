#
#CONFIG += debug_and_release
TEMPLATE = subdirs
CONFIG   += ordered
SUBDIRS  = rcada.pro \
	    rcada_client.pro 
#            lib/test2 \
#            lib/test 
#            lib/testscale

CFLAGS += -ffast-math
