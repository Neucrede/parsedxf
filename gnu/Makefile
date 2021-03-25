OUT = libparsedxf.so
OUTDIR = ./out
OBJDIR = ./obj
DEPDIR = ./deps
SRCDIR = ../src
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJS = $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(notdir $(SOURCES))))
DEPS = $(addprefix $(DEPDIR)/, $(patsubst %.c, %.d, $(notdir $(SOURCES))))
CC = gcc
CCFLAGS = -O2 -Wall -fPIC -DUSE_PTHREAD
LDFLAGS = --shared

INCLUDE_DEPS = 0

debug : deps main
debug : CCFLAGS += -DDEBUG
debug : INCLUDE_DEPS = 1

release : deps main
release : INCLUDE_DEPS = 1

main : $(OUT)

$(OUT) : $(OBJS)
	$(CC) -o $(OUT) $(OBJS) $(LDFLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $< -o $@

deps : $(DEPS)

ifeq ($(INCLUDE_DEPS),1)
include $(DEPS)
endif

$(DEPDIR)/%.d : $(SRCDIR)/%.c
	set -e; \
	rm -f $@; \
	$(CC) -MM -MP $(INCLUDE_DIRS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1\.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

.PHONY: clean
	
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(DEPDIR)/*.d

