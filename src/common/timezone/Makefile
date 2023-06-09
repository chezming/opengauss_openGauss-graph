#-------------------------------------------------------------------------
#
# Makefile
#    Makefile for the timezone library

# IDENTIFICATION
#    src/common/timezone/Makefile
#
#-------------------------------------------------------------------------

subdir = src/common/timezone
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

ifneq "$(MAKECMDGOALS)" "clean"
  ifneq "$(MAKECMDGOALS)" "distclean"
    ifneq "$(shell which g++ |grep hutaf_llt |wc -l)" "1"
      -include $(DEPEND)
    endif
  endif
endif
# files to build into backend
OBJS= localtime.o strftime.o pgtz.o

# files needed to build zic utility program
ZICOBJS= zic.o ialloc.o scheck.o localtime.o

# timezone data files
TZDATA = africa antarctica asia australasia europe northamerica southamerica \
	pacificnew etcetera factory backward systemv solar87 solar88 solar89
TZDATAFILES = $(TZDATA:%=$(srcdir)/data/%)

# which zone should determine the DST rules (not the specific UTC offset!)
# for POSIX-style timezone specs
POSIXRULES = US/Eastern

TMPVARS := $(LIBS)
LIBS = $(filter-out -ledit -lcrypt,$(TMPVARS))

# use system timezone data?
ifneq (,$(with_system_tzdata))
override CPPFLAGS += '-DSYSTEMTZDIR="$(with_system_tzdata)"'
endif

include $(top_srcdir)/src/gausskernel/common.mk

ifeq (,$(with_system_tzdata))
all: zic
endif

# We could do this test in the action section:
#	$(if $(ZIC),$(ZIC),./zic)
# but gmake versions <= 3.78.1 or perhaps later have a bug
# that causes a segfault;  gmake 3.81 or later fixes this.
ifeq (,$(ZIC))
ZIC= ./zic
endif

zic: $(ZICOBJS) | submake-libpgport
	$(CC) $(CFLAGS) $(ZICOBJS) $(LDFLAGS) $(LDFLAGS_EX) $(LIBS) -o $@$(X)

install: all installdirs
ifneq ($(with_openeuler_os), yes)
	cp $(LIBOPENSSL_LIB_PATH)/libssl.so* '$(DESTDIR)$(libdir)/'
	cp $(LIBOPENSSL_LIB_PATH)/libcrypto.so* '$(DESTDIR)$(libdir)/'
endif
ifeq (,$(with_system_tzdata))
	$(ZIC) -d '$(DESTDIR)$(datadir)/timezone' -p '$(POSIXRULES)' $(TZDATAFILES)
endif
	$(MAKE) -C tznames $@

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(datadir)'

uninstall:
	rm -f $(DESTDIR)$(libdir)/libssl.so*
	rm -f $(DESTDIR)$(libdir)/libcrypto.so*

ifeq (,$(with_system_tzdata))
	rm -rf '$(DESTDIR)$(datadir)/timezone'
endif
	$(MAKE) -C tznames $@

clean distclean maintainer-clean:
	rm -f zic$(X) $(ZICOBJS) *.depend
