
MAKE	=make
CC	=gcc
CXX	=g++
AR	=ar

CFLAGS	+=-Wall -g
CXXFLAGS+=-Wall -g -std=c++11 #-rdynamic
#ARFLAGS=cv

PRJ_HOME=..
PRJ_LIBS=-L$(PRJ_HOME)/dependencies/linux/libevent/lib -levent \
	-L$(PRJ_HOME)/dependencies/linux/libiconv/lib -liconv \
	-L$(PRJ_HOME)/dependencies/linux/log4cplus/lib -llog4cplus \
	-L/usr/lib64 -luuid \
	-lpthread -ldl -lm -lrt
	#-lunwind -lunwind-x86_64 -lpthread -ldl -lm -lrt

PRJ_INC	=-I$(PRJ_HOME)/include \
	-I$(PRJ_HOME)/dependencies/linux \
	-I$(PRJ_HOME)/dependencies/linux/libevent/include \
	-I$(PRJ_HOME)/dependencies/linux/libiconv/include \
	-I$(PRJ_HOME)/dependencies/linux/log4cplus/include

OBJDIR	=objs
BINDIR	=bins
LIBNAMES=xhnet
LIBNAME	=lib$(LIBNAMES).a

#makefile xie de bu zhadi, cou he yongyong ba 

SRCDIR1	=$(PRJ_HOME)/source/exception
SRCDIR2	=$(PRJ_HOME)/source/log
SRCDIR3	=$(PRJ_HOME)/source/md5
SRCDIR4	=$(PRJ_HOME)/source/memory
SRCDIR5	=$(PRJ_HOME)/source/net
SRCDIR6	=$(PRJ_HOME)/source/utility
TESTDIR	=$(PRJ_HOME)/test

LIBSRC	=$(wildcard $(SRCDIR1)/*.cpp) \
	$(wildcard $(SRCDIR2)/*.cpp) \
	$(wildcard $(SRCDIR3)/*.cpp) \
	$(wildcard $(SRCDIR4)/*.cpp) \
	$(wildcard $(SRCDIR5)/*.cpp) \
	$(wildcard $(SRCDIR6)/*.cpp)

TESTSRC	=$(wildcard $(TESTDIR)/*.cpp)

#LIBSRCNODIR =$(notdir $(LIBSRC))
#LIBOBJ	=$(LIBSRCNODIR:%.cpp=$(OBJDIR)/%.o)
LIBOFILE=$(notdir $(patsubst %.cpp,%.o,$(LIBSRC)))
LIBOBJ	=$(addprefix $(OBJDIR)/,$(LIBOFILE))

TESTOFILE=$(notdir $(patsubst %.cpp,%.o,$(LIBSRC)))
TESTOBJ	=$(addprefix $(OBJDIR)/,$(TESTOFILE))

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
$(OBJDIR)/%.o:$(TESTDIR)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(PRJ_INC) -o $@ $<

$(OBJDIR)/$(LIBNAME):$(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $< $(LIBOBJ)

$(BINDIR)/test:$(OBJDIR)/test.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)
$(BINDIR)/test-exception:$(OBJDIR)/test-exception.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)
$(BINDIR)/test-mpool:$(OBJDIR)/test-mpool.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)
$(BINDIR)/test-netbase:$(OBJDIR)/test-netbase.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)
$(BINDIR)/test-opool:$(OBJDIR)/test-opool.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)
$(BINDIR)/test-utility:$(OBJDIR)/test-utility.o
	$(CXX) -o $@ $< -L$(OBJDIR) -l$(LIBNAMES) $(PRJ_LIBS)

prepare:
	[[ -d $(OBJDIR) ]] || mkdir $(OBJDIR)
	[[ -d $(BINDIR) ]] || mkdir $(BINDIR)

$(BINDIR)/test:

test:prepare $(OBJDIR)/$(LIBNAME) $(BINDIR)/test $(BINDIR)/test-exception $(BINDIR)/test-mpool $(BINDIR)/test-netbase $(BINDIR)/test-opool $(BINDIR)/test-utility

all:prepare $(OBJDIR)/$(LIBNAME) 

clean:
	[[ -d $(OBJDIR) ]] & rm $(OBJDIR) -fr
	[[ -d $(BINDIR) ]] & rm $(BINDIR) -fr

.PHONY:clean test

