SRCDIR = ..
SOURCES = $(wildcard $(SRCDIR)/*.c)
INCDIRS = -I$(SRCDIR) -I../../src -I.
LIBDIRS = -L../../mingw
OBJDIR = ./obj
OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
OUTS = $(addprefix ./, $(patsubst %.o, %.exe, $(notdir $(OBJS))))
CC = gcc
CCFLAGS = -c -O2 -g $(INCDIRS) 
LDFLAGS = -lparsedxf $(LIBDIRS)

all : main

main : $(OUTS)
	-cp -f ../../mingw/libparsedxf.dll ./

%.exe : $(OBJDIR)/%.o
	$(CC) -o $@ $< $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) $< -o $@

.PHONY: clean

clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(DEPDIR)/*.d
	-rm -f $(OUTS)

	
