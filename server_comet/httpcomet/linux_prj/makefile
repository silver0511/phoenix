#编译版本debug,release,release_log
COMP_VER	= debug

#工程名
PROJECT	= httpcomet

#版本号，以后发布时候修改
include ./versionno.h
VERSION = $(DEFVERSION)



#debug  
ifeq ($(COMP_VER),debug)
LMLIBPATH	= ../../../server_bin/product_d
CXXFLAG      	=  -g -ggdb -Wall -D_DEBUG -DTRACE_LOG -D__SERVER -DUNICODE -DWEBSOCKETPP
OUTNAME		= $(PROJECT).d.$(VERSION)
LIBS		= ../../../server_bin/lib/libcommon.d.a \
		 		../../../server_bin/lib/libredisclient.d.a \
		 		../../../server_bin/lib/libhiredis.a \
		 		/usr/lib/boost/libboost_system.a \
		 		/usr/lib/boost/libboost_chrono.a
SOLIB		= -lnethttp.d  -lnettpool.d -lboost_system -lboost_chrono -lssl -lcrypto
endif

#release
ifeq ($(COMP_VER),release)
LMLIBPATH	= ../../../server_bin/product
CXXFLAG      	=  -w -O3  -DNDEBUG -D__SERVER -DUNICODE -DWEBSOCKETPP
OUTNAME		= $(PROJECT).$(VERSION)
LIBS		= ../../../server_bin/lib/libcommon.a \
		 		../../../server_bin/lib/libredisclient.a \
		 		../../../server_bin/lib/libhiredis.a \
                /usr/lib/boost/libboost_system.a \
                /usr/lib/boost/libboost_chrono.a
SOLIB		=  -lnethttp -lnettpool -lboost_system -lboost_chrono -lssl -lcrypto

endif

#release
ifeq ($(COMP_VER),release_log)
LMLIBPATH	= ../../../server_bin/product_l
CXXFLAG      	=  -w -O3  -DTRACE_LOG -DNDEBUG -D__SERVER -DUNICODE -DWEBSOCKETPP
OUTNAME		= $(PROJECT).l.$(VERSION) 
LIBS		= ../../../server_bin/lib/libcommon.l.a \
		 		../../../server_bin/lib/libredisclient.l.a \
		 		../../../server_bin/lib/libhiredis.a
SOLIB		=  -lnethttp.l  -lnettpool.l -lboost_system -lboost_chrono -lssl -lcrypto
endif

#---------------------------------------------------------
ARCH       	= linux
SrcSuf       	= c
SrcSuf2		= cpp
ObjSuf      	= o 
LibSuf       	= a
#---------------------------------------------------------
ifeq ($(ARCH),linux)	

CXX           	= g++ 
CXXFLAGS        = -std=c++0x -w  -pthread -fshort-wchar  -fPIC -rdynamic $(CXXFLAG)

#libary path
LIBPATH		=  -L../../../server_bin/lib -L$(LMLIBPATH) -L/usr/lib/boost

#include path
INCLUDEPATH	=   -I../../../Include -I../../../Include/Package/hfbs/common -I../Src -I../../../public/server_common
	                          
	          
		
LD     		= g++
LDFLAGS		= -std=c++0x -lpthread $(LIBPATH)  $(SOLIB)

endif

#---------------------------------------------------------
OBJFILES		=	../Src/Config.$(ObjSuf)\
		  			../Src/GlobalResource.$(ObjSuf)\
		  			../Src/Server.$(ObjSuf)\
		  			../Src/ServerMgr.$(ObjSuf)\
					../Src/ServerMgrTimeTask.$(ObjSuf)\
					../Src/stdafx.$(ObjSuf)\
					../../../public/server_common/ClientConn/User.$(ObjSuf)\
					../../../public/server_common/ClientConn/UserMgr.$(ObjSuf)\
					../Src/ClientSide/ClientSide.$(ObjSuf)\
					../Src/ServerSide/ServerSide.$(ObjSuf)\
			

#---------------------------------------------------------
.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(LibSuf) .$(SrcSuf2)

all:  $(PROJECT) install clean

$(PROJECT):   $(OBJFILES)
	@echo "Linking $(OUTNAME) start..." 
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(OUTNAME) -ldl
	@echo "Linking $(OUTNAME) end"

clean:
	@echo "Cleaning $(PROJECT) project files"
	@rm -f $(OBJFILES) core
	@rm -f ./*.a
	
install:
    ifeq ($(COMP_VER),debug)				
		@echo "copying $(OUTNAME) to  ../../../server_bin/product_d/"
		@cp $(OUTNAME) ../../../server_bin/product_d/
		@echo "all process completed."
    endif 
    
    ifeq ($(COMP_VER),release)				
		@echo "copying $(OUTNAME) to  ../../../server_bin/product/"
		@cp $(OUTNAME) ../../../server_bin/product/
		@echo "all process completed."
    endif 
    
    ifeq ($(COMP_VER),release_log)				
		@echo "copying $(OUTNAME) to ../../../server_bin/product_l/"
		@cp $(OUTNAME) ../../../server_bin/product_l/
		@echo "all process completed."
    endif 
    
#---------------------------------------------------------
.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $(PROJECT) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 

#---------------------------------------------------------
.$(SrcSuf2).$(ObjSuf):
	@echo "Compiling $(PROJECT) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 



