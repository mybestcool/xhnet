
MAKE	=make
CC	=gcc
CXX	=g++
AR	=ar

CFLAGS	+=-Wall
CXXFLAGS+=-Wall -std=c++0x 
#ARFLAGS=cv

PRJ_HOME=..
PRJ_LIBS=-L$(PRJ_HOME)/dependencies/linux/libevent/lib -levent \
	-L$(PRJ_HOME)/dependencies/linux/libiconv/lib -liconv \
	-L$(PRJ_HOME)/dependencies/linux/log4cplus/lib -llog4cplus \
	-L/usr/lib64 -luuid \
	-lpthread

PRJ_INC	=-I$(PRJ_HOME)/include \
	-I$(PRJ_HOME)/dependencies/linux \
	-I$(PRJ_HOME)/dependencies/linux/libevent/include \
	-I$(PRJ_HOME)/dependencies/linux/libiconv/include \
	-I$(PRJ_HOME)/dependencies/linux/log4cplus/include 

OBJDIR	=objs
LIBNAME	=libxhnet.a

#makefile xie de bu zhadi, cou he yongyong ba 

SRCDIR1	=$(PRJ_HOME)/source/exception
SRCDIR2	=$(PRJ_HOME)/source/log
SRCDIR3	=$(PRJ_HOME)/source/md5
SRCDIR4	=$(PRJ_HOME)/source/memory
SRCDIR5	=$(PRJ_HOME)/source/net
SRCDIR6	=$(PRJ_HOME)/source/utility

LIBSRC	=$(wildcard $(SRCDIR1)/*.cpp) \
	$(wildcard $(SRCDIR2)/*.cpp) \
	$(wildcard $(SRCDIR3)/*.cpp) \
	$(wildcard $(SRCDIR4)/*.cpp) \
	$(wildcard $(SRCDIR5)/*.cpp) \
	$(wildcard $(SRCDIR6)/*.cpp)

#LIBSRCNODIR =$(notdir $(LIBSRC))
#LIBOBJ	=$(LIBSRCNODIR:%.cpp=$(OBJDIR)/%.o)
LIBOFILE=$(notdir $(patsubst %.cpp,%.o,$(LIBSRC)))
LIBOBJ	=$(addprefix $(OBJDIR)/,$(LIBOFILE))


$(OBJDIR)/%.o:$(SRCDIR1)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<
$(OBJDIR)/%.o:$(SRCDIR2)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<
$(OBJDIR)/%.o:$(SRCDIR3)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<
$(OBJDIR)/%.o:$(SRCDIR4)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<
$(OBJDIR)/%.o:$(SRCDIR5)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<
$(OBJDIR)/%.o:$(SRCDIR6)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<

$(OBJDIR)/$(LIBNAME):$(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $(LIBOBJ)

prepare:
	[[ -d $(OBJDIR) ]] || mkdir $(OBJDIR)

all:prepare $(OBJDIR)/$(LIBNAME) 

clean:
	[[ -d $(OBJDIR) ]] & rm $(OBJDIR) -fr

.PHONY:clean test

