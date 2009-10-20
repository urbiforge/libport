## Create the .version file.
##
## This Makefile is not meant to processed by Automake, which dislikes
## the tricks we play with VERSION and so forth.  So include it in a
## GNUmakefile.  Which also explains why we have to ship it
## "manually".
EXTRA_DIST +=					\
  build-aux/git-version-gen			\
  build-aux/package-version.mk

# .version
VERSION_FILE = $(top_srcdir)/.version

# Ship it, so that when we are detached from a repository, we still
# have it available.  Besides, since "dist" depends on "EXTRA_DIST",
# it ensures that each time we run "make dist", ".version" is updated,
# which is what we want.  Having "all" depend on it would recreate it
# too often.
EXTRA_DIST += $(VERSION_FILE) $(VERSION_FILE).stamp

# all: $(VERSION_FILE) $(VERSION_FILE).stamp

# The version to use in the Makefile to create the tarball.
VERSION =								\
  $(shell sed -n '/^TarballVersion: /{s///;p;q;}' $(VERSION_FILE)	\
          || echo "UNKNOWN")

GIT_VERSION_GEN = $(build_aux_dir)/git-version-gen

# Update $(VERSION_FILE).stamp.
#
# We have a nasty problem here: it seems that GNU Make does not behave
# the same way on all hosts (especially GNU/Linux vs. OSX).  As a rule
# of thumb, files once qualified with $(top_srcdir) should always be.
# But in my case (Akim), VPATH is ../.., and it turns out I also have
# a package in ../../$(top_srcdir), so it actually looks in
# ../../../..  and finds a .version.stamp there :( I don't know what
# to do.
#
# The BF seems to prefer the solution below.
$(VERSION_FILE).stamp: $(GIT_VERSION_GEN)
	@rm -f $@.tmp
	@$(mkdir_p) $(dir $@)
	@touch $@.tmp
	if test ! -f $(VERSION_FILE)			\
	   || test -n "$$BUILDBOT_ROOT"; then		\
	  $(GIT_VERSION_GEN)				\
		--srcdir=$(top_srcdir)			\
		--cache=$(VERSION_FILE);		\
	fi
	@mv -f $@.tmp $@

$(VERSION_FILE): $(VERSION_FILE).stamp
	@if test -f $(VERSION_FILE); then :; else	\
	  rm -f $(VERSION_FILE).stamp;			\
	  $(MAKE) $(AM_MAKEFLAGS) $<;			\
	fi

# Force the update of the file if its value differs.
.PHONY: update-version
update-version:
	rm -f $(VERSION_FILE).stamp

.PHONY: debug debug-version
debug: debug-version
debug-version:
	@echo "\$$(VERSION) = $(VERSION)"
