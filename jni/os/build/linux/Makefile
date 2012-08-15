# Inside the module specific make file, No need to do much. Just define
# ROOTDIR - pointing to the root path where the Makefile.include and Makefile.Cleanup is present.
# TARGETLIB - define the name of the target lib if you want to compile this as module otherwise
# TARGETEXE - define the name of the target executable if you want to compile this as an executable.
# SOURCES -  define all the soucrce files here.
# LIBDIRS - Only needed when your target is executable. define the directories of the libs to be linked are present.
#	    Lib directory inclusion should be in the form "-L../../common/os/linux/output/ -L../../socketimpl/linux/output/"
# LIBS - Only needed when your target is executable. Just define the libs which need to be linked..
#	    Libs inclusion should be in the form "-lsocket -llinuxos -lpthread"
# That's it. Enjoy.....:)

ROOTDIR = ../../../

TARGETLIB = $(OUTPUTDIR)libos_linux.a

INCLUDE_DIRS +=
LINUXDIR = $(ROOTDIR)os/linux/

LIB_SOURCES = $(LINUXDIR)linux_os.cpp \
	      $(LINUXDIR)../os.cpp

SOURCES = $(LIB_SOURCES)

include $(ROOTDIR)Makefile.include
