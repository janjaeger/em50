#

bin := em50

prefix := /usr/local

CFLAGS := -O3 -Wall -std=gnu11
LDFLAGS := -lreadline -lpthread -lm -ltelnet

instdir := $(DESTDIR)$(prefix)/bin

src := $(wildcard *.c)
obj := $(src:.c=.o)
dep := $(obj:.o=.d)

MAKEFILE := $(MAKEFILE_LIST)

.PHONY: all clean install uninstall distclean

all: $(bin)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(dep)
endif

%.d: %.c $(MAKEFILE)
	@$(CC) -MM -M $< $(CFLAGS) | sed 's,\($*\)\.o[:]*,\1.o $@:,g' > $@

%.o: %.c $(MAKEFILE)
	@$(CC) -o $@ -c $< $(CFLAGS)

$(bin): $(obj)
	@$(CC) -o $@ $^ $(LDFLAGS)

install: $(bin)
	@install -d $(instdir)
	@install $< $(instdir)

uninstall:
	@$(RM) $(instdir)/$(bin)

clean: 
	@$(RM) -rf $(bin) $(obj) $(dep)

distclean: clean
