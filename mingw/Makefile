OUT = libparsedxf.dll
OUTA = $(OUT:.dll=.a)
OUTDIR = ./out
OBJDIR = ./obj
DEPDIR = ./deps
SRCDIR = ../src
INCDIRS = -I$(SRCDIR)
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
DEPS = $(addprefix $(DEPDIR)/, $(patsubst %.c, %.d, $(notdir $(SOURCES))))
CC = gcc
CCFLAGS = -Wall -Wno-unused-variable -Wno-unused-function \
		  -fPIC -D_POSIX_C_SOURCE $(INCDIRS)
LDFLAGS = --shared
AR = ar
ARFLAGS = rsv

INCLUDE_DEPS = 0

debug : deps main
debug : CCFLAGS += -g -DDEBUG
debug : INCLUDE_DEPS = 1

release : deps main
release : CCFLAGS += -O2
release : INCLUDE_DEPS = 1

main : $(OUT) $(OUTA)

$(OUT) : $(OBJS)
	$(CC) -o $(OUT) $(OBJS) $(LDFLAGS)
	
$(OUTA) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $< -o $@

deps : $(DEPS)

ifeq ($(INCLUDE_DEPS),1)
include $(DEPS)
endif

$(DEPDIR)/%.d : $(SRCDIR)/%.c
	set -e; \
	rm -f $@; \
	$(CC) -MM -MP $(INCDIRS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1\.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: clean
	
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(DEPDIR)/*.d
	-rm -f $(OUT)
	-rm -f $(OUTA)
