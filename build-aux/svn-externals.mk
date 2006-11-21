# To use this file, accumulate in 
#
# - SVN_EXTERNALS
#   the list of directories names that are svn:externals.
#
# - SVN_EXTERNALS_PROXY
#   the list of directories names that are svn:externals in sub
#   directories, and for which there exists a local set of
#   proxy commands.  For an example of proxy targets, see
#   kernel1/Makefile.am.

# This is the only place we initialize SVN_EXTERNALS and
# SVN_EXTERNALS_PROXY, this helps making sure the file is not included
# twice, and at least once (otherwise Automake complains about it not
# being defined).
SVN_EXTERNALS =
SVN_EXTERNALS_PROXY =

SVN = svn
svn_externals = $(top_srcdir)/build-aux/svn-externals

# I would like to get rid of this, but Automake is too conservative
# when parsing rules, and rejects (an error) targets like
# $(SVN_EXTERNALS:=-up), but accepts $(SVN_EXTERNALS:=.up) (a period).
SVN_EXTERNALS_CI   = $(SVN_EXTERNALS:=-ci)
SVN_EXTERNALS_HELP = $(SVN_EXTERNALS:=-help)
SVN_EXTERNALS_PIN  = $(SVN_EXTERNALS:=-pin)
SVN_EXTERNALS_UP   = $(SVN_EXTERNALS:=-up)

.PHONY: $(SVN_EXTERNALS_HELP) $(SVN_EXTERNALS_PIN) \
        $(SVN_EXTERNALS_CI)   $(SVN_EXTERNALS_UP)
help: $(SVN_EXTERNALS_HELP)
$(SVN_EXTERNALS_HELP):
	@ext=$(@:-help=);						 \
	echo "$$ext-up:    update the svn:externals revision for $$ext"; \
	echo "$$ext-ci:    check in $$ext and up it";			 \
	echo "$$ext-pin:   subscribe to the latest $$ext"

# Update the pinned external.
$(SVN_EXTERNALS_UP):
	$(svn_externals) --update=$(@:-up=) $(srcdir)

# Checkin the pinned external and update it.
$(SVN_EXTERNALS_CI):
	cd $(srcdir)/$(@:-ci=) && $(SVN) ci
	$(MAKE) $(AM_MAKEFLAGS) $(@:-ci=-up)

# Pin the svn:external by subscribing to it.
$(SVN_EXTERNALS_PIN):
	$(svn_externals) --subscribe=$(@:-pin=) $(srcdir)


## ------------------- ##
## All svn:externals.  ##
## ------------------- ##

.PHONY: externals-up-help externals-up externals-ci

help: externals-help
externals-help:
	@echo "externals-ci:    check in all the svn:externals dependencies"
	@echo "externals-up:    update all the svn:externals dependencies"

# Externals at once.
externals-ci: $(SVN_EXTERNALS_CI) $(SVN_EXTERNALS_PROXY:=-ci)
externals-up: $(SVN_EXTERNALS_UP) $(SVN_EXTERNALS_PROXY:=-up)
