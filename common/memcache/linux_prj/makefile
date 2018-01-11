#编译版本debug,release,release_log
COMP_VER	=debug

#工程名
PROJECT	= libmemcache

#debug  
ifeq ($(COMP_VER),debug)				
CXXFLAG      	=  -g -ggdb -Wall -D_DEBUG -DTRACE_LOG -DUNICODE -D__SERVER -D _GLIBCXX_DEPRECATED
OUTNAME		= $(PROJECT).d
endif

#release
ifeq ($(COMP_VER),release)				
CXXFLAG      	= -g -w -O3  -DNDEBUG -DUNICODE -D__SERVER -D _GLIBCXX_DEPRECATED
OUTNAME		=  $(PROJECT)
endif

#release_log
ifeq ($(COMP_VER),release_log)				
CXXFLAG      	= -g -w -O3 -DNDEBUG  -DTRACE_LOG -DUNICODE -D__SERVER -D _GLIBCXX_DEPRECATED
OUTNAME		=  $(PROJECT).l
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
CXXFLAGS        = -w -fPIC  -fshort-wchar -D__cplusplus $(CXXFLAG)  
LIBPATH		= -L ../../../server_bin/lib
INCLUDEPATH	= -I../../../Include -I../../../Include/common/
LD     		= ar 
LDFLAGS  	= rcs 
endif

#---------------------------------------------------------
OBJFILES	= 	../src/BaseMemCache.$(ObjSuf)
				
				
#---------------------------------------------------------
.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(LibSuf) .$(SrcSuf2)

all:  $(PROJECT)
$(PROJECT):$(OBJFILES)
	@echo "Linking $(PROJECT) start..." 
	$(LD) $(LDFLAGS) $(OUTNAME).$(LibSuf) $(OBJFILES)
	@echo "Linking $(PROJECT) end"

clean:
	@echo "Cleaning $(PROJECT) project files"
	@rm -f $(OBJFILES) core
	@rm -f ./*.a
	
install:
	@echo *.a ../../../server_bin/lib/
	cp $(OUTNAME).a ../../../server_bin/lib/
#---------------------------------------------------------
.$(SrcSuf).$(ObjSuf):
	@echo "Compiling $(PROJECT) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 

#---------------------------------------------------------
.$(SrcSuf2).$(ObjSuf):
	@echo "Compiling $(OUTNAME) $<" 
	$(CXX) -c $(CXXFLAGS) $(INCLUDEPATH) $(LIBPATH) $< -o $@ 


