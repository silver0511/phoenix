#
#Copyright 2011- 360Youjiu(Beijing) Technology Co.Ltd. All Rights Reserved
#
# tcppoolcore make文件
# 需要在../../../server_bin/lib目录下执行连接程序
# ln -s libtcppoolcore.x.x.x.so libtcppoolcore.so
#---------------------------------------------------------
#编译版本debug,release,release_log
COMP_VER	=debug

#工程名
PROJECT	= libnettpool




#版本号，以后发布时候修改
include ./versionno.h
VERSION = $(DEFVERSION)
	

#debug  
ifeq ($(COMP_VER),debug)		
#lm库文件路径
LMLIBPATH	= ../../../server_bin/product_d
CXXFLAG     = -g -ggdb -Wall -D_DEBUG -DTRACE_LOG

OUTNAME		= $(PROJECT).d.$(VERSION)
endif

#release
ifeq ($(COMP_VER),release)
#lm库文件路径
LMLIBPATH	= ../../../server_bin/product
CXXFLAG     = -g -w -O3  -DNDEBUG 
OUTNAME		= $(PROJECT).$(VERSION)

endif

#release
ifeq ($(COMP_VER),release_log)
#lm库文件路径
LMLIBPATH	= ../../../server_bin/product_l
CXXFLAG     = -g -w -O3  -DTRACE_LOG -DNDEBUG  
OUTNAME		= $(PROJECT).l.$(VERSION)
endif



#---------------------------------------------------------
ARCH       	= linux
SrcSuf      = c
SrcSuf2		= cpp
ObjSuf      = o 
LibSuf      = a
#---------------------------------------------------------
ifeq ($(ARCH),linux)	

CXX         = g++ 
CXXFLAGS    = -std=c++0x -w -pthread -fshort-wchar  -fPIC -rdynamic $(CXXFLAG) -DAPI_EXPORTS -D__SERVER -DUNICODE

#libary path
LIBPATH		= -L../../../server_bin/lib

#include path
INCLUDEPATH	= -I../../../Include \
							-I../src/  \
							-I../prj
LD     		= g++
LDFLAGS		= -fPIC -shared -lpthread 

endif

#---------------------------------------------------------
OBJFILES		=	../src/TCPConnectPool.$(ObjSuf)\
					../src/TCPConnPoolMgr.$(ObjSuf)\
					../prj/DLLExport.$(ObjSuf)\
                    ../../common_tcp/ServerPack.$(ObjSuf)  \
#---------------------------------------------------------
.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(LibSuf) .$(SrcSuf2)

all:  $(PROJECT) 

$(PROJECT):   $(OBJFILES)
	@echo "Linking $(OUTNAME) start..." 
	$(LD) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $(OUTNAME).so
	@echo "Linking $(PROJECT) end"

clean:
	@echo "Cleaning $(PROJECT) files"
	@rm -f $(OBJFILES) core
	@rm -f ./*.a

install:
	@echo install $(PROJECT).so
	 
        # debug output
        ifeq ($(COMP_VER),debug)
		rm -f ../../../server_bin/product_d/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(PROJECT).d.so
	    cp $(OUTNAME).so ../../../server_bin/product_d/$(OUTNAME).so
	    cp $(OUTNAME).so $(LMLIBPATH)/$(OUTNAME).so
	    ln -s $(OUTNAME).so $(LMLIBPATH)/$(PROJECT).d.so
        endif

        # release output
        ifeq ($(COMP_VER),release)
	    rm -f ../../../server_bin/product/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(PROJECT).so
	    cp $(OUTNAME).so ../../../server_bin/product/$(OUTNAME).so
	    cp $(OUTNAME).so $(LMLIBPATH)/$(OUTNAME).so
	    ln -s $(OUTNAME).so $(LMLIBPATH)/$(PROJECT).so
        endif

        # release log output
        ifeq ($(COMP_VER),release_log)
	    rm -f ../../../server_bin/product_l/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(OUTNAME).so
	    rm -f $(LMLIBPATH)/$(PROJECT).l.so
	    cp $(OUTNAME).so ../../../server_bin/product_l/$(OUTNAME).so
	    cp $(OUTNAME).so $(LMLIBPATH)/$(OUTNAME).so
	    ln -s $(OUTNAME).so $(LMLIBPATH)/$(PROJECT).l.so
        endif
    
#---------------------------------------------------------
.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $(OUTNAME) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 

#---------------------------------------------------------
.$(SrcSuf2).$(ObjSuf):
	@echo "Compiling $(OUTNAME) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 
