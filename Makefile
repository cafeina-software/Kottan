## Haiku Generic Makefile v2.6 ##


NAME = Kottan
TYPE = APP
APP_MIME_SIG = application/x-vnd.BlueSky-Kottan

#%{
# @src->@
SRCS = \
	 src/editview.cpp  \
	 src/editwindow.cpp  \
	 src/app.cpp  \
	 src/datawindow.cpp  \
	 src/mainwindow.cpp  \
	 src/messageview.cpp  \
	 src/whatwindow.cpp  \
	 src/importerwindow.cpp  \
	 src/gettype.cpp  \
	 src/visualwindow.cpp  \
	 src/msginfowindow.cpp \

RDEFS = \
	 Kottan.rdef  \


RSRC = \

# @<-src@
#%}

#%}

#%}

#%}

#%}

#%}

#%}

#%}

LIBS = $(STDCPPLIBS) be root localestub columnlistview tracker shared bnetapi
LIBPATHS =
SYSTEM_INCLUDE_PATHS = /boot/system/develop/headers/private/interface
LOCAL_INCLUDE_PATHS =
OPTIMIZE :=
LOCALES = en de nl ru ca ro es sv tr
DEFINES=
WARNINGS = ALL
SYMBOLS := TRUE
DEBUGGER := TRUE
COMPILER_FLAGS = -std=c++11
LINKER_FLAGS =
APP_VERSION :=

DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

