# This Makefile is a trimmed version of the Linux kernel makefile for version
# 2.6.34. In a nutshell, all core, driver, and library targets are built as
# they are in Linux. The resulting object files and archives are available as
# $(vmlinux-all). This Makefile adds another rule to build $(PROGRAM)
# which takes $(vmlinux-all) as input files for linking.
#
# Some of the important differences are:
# - Everything will be compiled and linked into $(PROGRAM)
# - Removal of external module and firmware build support.
# - Removal of a lot of targets which used helper scripts like checkstack,
#    patch-kernel, etc.
# - Removed documentation targets (may wish to reconsider...)
# - Removed unused goals, like net-y/init-m and init-y/init-m
# - Add standard include files back in (NOSTDINC undefined)
# - Removed outputmakefile and other stuff for outputting to a different
#   directory

NAME="Mosys"
PROGRAM=mosys
TESTPROGRAM=$(PROGRAM)_test

# Mosys will use the following version format: core.major.minor-revision
# Here is a summery of each of those fields:
# core:		Mosys core architecture version
# major:	Major release number; incremented at major milestones
# minor:	Minor release number; incremented for important changes
# revision:	Patch number from version control system
CORE		=  1
MAJOR		=  2
MINOR		= 03
GITVERSION	:= $(shell ./scripts/getversion.sh -r)
TIMESTAMP 	:= $(shell ./scripts/getversion.sh -t)

RELEASENAME := "$(CORE).$(MAJOR).$(MINOR) : $(GITVERSION) : $(TIMESTAMP)"

# location to use when releasing new packages
export EXPORTDIR	?= .
export INSTALL_PATH	?= /usr/sbin

# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.

# Do not:
# o  use make's built-in rules and variables
#    (this increases performance and avoids hard-to-debug behaviour);
# o  print "Entering directory ...";
MAKEFLAGS += -rR --no-print-directory

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC

# We are using a recursive build, so we need to do a little thinking
# to get the ordering right.
#
# Most importantly: sub-Makefiles should only ever modify files in
# their own directory. If in some directory we have a dependency on
# a file in another dir (which doesn't happen often, but it's often
# unavoidable when linking the built-in.o targets which finally
# turn into vmlinux), we will call a sub make in that other dir, and
# after that we are sure that everything which is in that other dir
# is now up to date.
#
# The only cases where we need to modify files which have global
# effects are thus separated out and done before the recursive
# descending is started. They are now explicitly listed as the
# prepare rule.

# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

# Call a source code checker (by default, "sparse") as part of the
# C compilation.
#
# Use 'make C=1' to enable checking of only re-compiled files.
# Use 'make C=2' to enable checking of *all* source files, regardless
# of whether they are re-compiled or not.
#
# See the file "Documentation/sparse.txt" for more details, including
# where to get the "sparse" utility.

ifeq ("$(origin C)", "command line")
  KBUILD_CHECKSRC = $(C)
endif
ifndef KBUILD_CHECKSRC
  KBUILD_CHECKSRC = 0
endif

# We process the rest of the Makefile if this is the final invocation of make
ifeq ($(skip-makefile),)

PHONY += all
_all: all

srctree		:= $(if $(KBUILD_SRC),$(KBUILD_SRC),$(CURDIR))
objtree		:= $(CURDIR)
src		:= $(srctree)
obj		:= $(objtree)

VPATH		:= $(srctree)

export srctree objtree VPATH

# SUBARCH tells the usermode build what the underlying arch is.  That is set
# first, and if a usermode build is happening, the "ARCH=um" on the command
# line overrides the setting of ARCH below.  If a native build is happening,
# then ARCH is assigned, getting whatever value it gets normally, and 
# SUBARCH is subsequently ignored.

SUBARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/s390x/s390/ -e s/parisc64/parisc/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/sh[234].*/sh/ )

# Cross compiling and selecting different set of gcc/bin-utils
# ---------------------------------------------------------------------------
#
# When performing cross compilation for other architectures ARCH shall be set
# to the target architecture. (See arch/* for the possibilities).
# ARCH can be set during invocation of make:
# make ARCH=ia64
# Another way is to have ARCH set in the environment.
# The default ARCH is the host where make is executed.

# CROSS_COMPILE specify the prefix used for all executables used
# during compilation. Only gcc and related bin-utils executables
# are prefixed with $(CROSS_COMPILE).
# CROSS_COMPILE can be set on the command line
# make CROSS_COMPILE=ia64-linux-
# Alternatively CROSS_COMPILE can be set in the environment.
# Default value for CROSS_COMPILE is not to prefix executables
# Note: Some architectures assign CROSS_COMPILE in their arch/*/Makefile
export KBUILD_BUILDHOST := $(SUBARCH)
ARCH		?= $(SUBARCH)
CROSS_COMPILE	?=

# Architecture as present in compile.h
SRCARCH 	:= $(ARCH)

# Additional ARCH settings for x86
ifeq ($(ARCH),i386)
        SRCARCH := x86
endif
ifeq ($(ARCH),x86_64)
        SRCARCH := x86
endif

# Additional ARCH settings for sparc
ifeq ($(ARCH),sparc64)
       SRCARCH := sparc
endif

# Additional ARCH settings for sh
ifeq ($(ARCH),sh64)
       SRCARCH := sh
endif

# Where to locate arch specific headers
hdr-arch  := $(SRCARCH)

ifeq ($(ARCH),m68knommu)
       hdr-arch  := m68k
endif

KCONFIG_CONFIG	?= .config

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

# FIXME: CFLAGS must have a -O option due to usage of inb/outb and similar
# functions on x86. See outb man page for details.
HOSTCC       ?= gcc
HOSTLD       ?= ld
HOSTCXX      ?= g++
HOSTCFLAGS   ?= -Wall -Wmissing-prototypes -Wstrict-prototypes -Os -fomit-frame-pointer
HOSTCXXFLAGS ?= -Os

# Decide whether to build built-in, modular, or both.
# Normally, just do built-in.

KBUILD_BUILTIN := 1

#	If we have only "make modules", don't compile built-in objects.
#	When we're building modules with modversions, we need to consider
#	the built-in objects during the descend as well, in order to
#	make sure the checksums are up to date before we record them.

ifeq ($(MAKECMDGOALS),modules)
  KBUILD_BUILTIN := $(if $(CONFIG_MODVERSIONS),1)
endif

export KBUILD_BUILTIN
export KBUILD_CHECKSRC KBUILD_SRC

# Beautify output
# ---------------------------------------------------------------------------
#
# Normally, we echo the whole command before executing it. By making
# that echo $($(quiet)$(cmd)), we now have the possibility to set
# $(quiet) to choose other forms of output instead, e.g.
#
#         quiet_cmd_cc_o_c = Compiling $(RELDIR)/$@
#         cmd_cc_o_c       = $(CC) $(c_flags) -c -o $@ $<
#
# If $(quiet) is empty, the whole command will be printed.
# If it is set to "quiet_", only the short version will be printed. 
# If it is set to "silent_", nothing will be printed at all, since
# the variable $(silent_cmd_cc_o_c) doesn't exist.
#
# A simple variant is to prefix commands with $(Q) - that's useful
# for commands that shall be hidden in non-verbose mode.
#
#	$(Q)ln $@ :<
#
# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(findstring s,$(MAKEFLAGS)),)
  quiet=silent_
endif

export quiet Q KBUILD_VERBOSE

UNITTEST		?= n
CMOCKERY_PATH		:= tools/cmockery
CMOCKERY_INCLUDE	:= -I$(CMOCKERY_PATH)/src/google
CMOCKERY_FIND_IGNORE	:= \( -name cmockery \) -prune -o
UNITTEST_DATA          := $(addsuffix /tools/test_data/, $(shell pwd))
GENHTML_OUTPUT_DIR	:= html

# Look for make include files relative to root of kernel src
MAKEFLAGS += --include-dir=$(srctree)

# We need some generic definitions (do not try to remake the file).
$(srctree)/scripts/Kbuild.include: ;
include $(srctree)/scripts/Kbuild.include

# Make variables (CC, etc...)

AR		?= $(CROSS_COMPILE)ar
AS		?= $(CROSS_COMPILE)as
CC		?= $(CROSS_COMPILE)gcc
CPP		?= $(CC) -E
LD		?= $(CROSS_COMPILE)ld
NM		?= $(CROSS_COMPILE)nm
STRIP		?= $(CROSS_COMPILE)strip
OBJCOPY		?= $(CROSS_COMPILE)objcopy
OBJDUMP		?= $(CROSS_COMPILE)objdump
INSTALL		?= install
AWK		= awk
DEPMOD		= /sbin/depmod
CHECK		= sparse

CHECKFLAGS     := -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
		  -Wbitwise -Wno-return-void $(CF)
#CFLAGS_KERNEL	=
#AFLAGS_KERNEL	=
CFLAGS_GCOV	:= -fprofile-arcs -ftest-coverage -lgcov

# Use LINUXINCLUDE when you must reference the include/ directory.
# Needed to be compatible with the O= option
LINUXINCLUDE    := -Iinclude \
                   -include include/generated/autoconf.h \
                   -Itools/vpd_encode

KERNELVERSION	= $(CORE).$(MAJOR).$(MINOR)

FMAP_LINKOPT	?= -lfmap-0.3
LDLIBS		:= -luuid $(FMAP_LINKOPT)

#EXTRA_CFLAGS	:= $(patsubst %,-l%, $(LIBRARIES))

MOSYS_MACROS	:= -DPROGRAM=\"$(PROGRAM)\" \
		   -DVERSION=\"$(RELEASENAME)\"

KBUILD_CPPFLAGS := 

KBUILD_CFLAGS   := -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs \
		   -fno-strict-aliasing -fno-common \
		   -Werror-implicit-function-declaration \
		   -Wno-format-security \
		   -fno-delete-null-pointer-checks
KBUILD_AFLAGS   := -D__ASSEMBLY__

export VERSION PATCHLEVEL SUBLEVEL KERNELRELEASE KERNELVERSION UNITTEST
export MOSYS_MACROS EXTRA_CFLAGS LIBRARIES
export ARCH SRCARCH CONFIG_SHELL HOSTCC HOSTCFLAGS HOSTLD HOSTLDFLAGS \
       CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP
export MAKE AWK INSTALLKERNEL
export HOSTCXX HOSTCXXFLAGS CHECK CHECKFLAGS

export KBUILD_CPPFLAGS LINUXINCLUDE OBJCOPYFLAGS LDFLAGS
export KBUILD_CFLAGS CFLAGS_KERNEL CFLAGS_GCOV
export KBUILD_AFLAGS AFLAGS_KERNEL

# Files to ignore in find ... statements
RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o -name .svn -o -name CVS -o -name .pc -o -name .hg -o -name .git \) -prune -o
export RCS_TAR_IGNORE := --exclude SCCS --exclude BitKeeper --exclude .svn --exclude CVS --exclude .pc --exclude .hg --exclude .git

# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

no-dot-config-targets := clean mrproper distclean \
			 cscope TAGS tags help %docs check% \
			 include/linux/version.h headers_% \
			 kernelrelease kernelversion

config-targets := 0
mixed-targets  := 0
dot-config     := 1

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifneq ($(filter config %config,$(MAKECMDGOALS)),)
        config-targets := 1
        ifneq ($(filter-out config %config,$(MAKECMDGOALS)),)
                mixed-targets := 1
        endif
endif

ifeq ($(mixed-targets),1)
# ===========================================================================
# We're called with mixed targets (*config and build targets).
# Handle them one by one.

%:: FORCE
	$(Q)$(MAKE) -C $(srctree) KBUILD_SRC= $@

else
ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

# Default configuration stuff used for 'make defconfig'
include $(srctree)/configs/Makefile

KBUILD_KCONFIG	:= Kconfig	# Use top-level Kconfig for now.
export KBUILD_DEFCONFIG KBUILD_KCONFIG

config: scripts_basic FORCE
	$(Q)mkdir -p include/linux include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

%config: scripts_basic FORCE
	$(Q)mkdir -p include/linux include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@

else
# ===========================================================================
# Build targets only - this includes vmlinux, arch specific targets, clean
# targets and others. In general all targets except *config targets.

# Additional helpers built in scripts/
# Carefully list dependencies so we do not try to build scripts twice
# in parallel
PHONY += scripts
scripts: scripts_basic include/config/auto.conf include/config/tristate.conf
	$(Q)$(MAKE) $(build)=$(@)

# Objects we will link into mosys / subdirs we need to visit
#drivers-y	:= drivers/ sound/ firmware/
#libs-y		:= lib/
#core-y		:= usr/
drivers-y	:=
libs-y		:=
tools-y		:= tools/
#FIXME: Clean up core-y
core-y		:= core/ intf/ drivers/ platform/ lib/ $(tools-y)

# Tools that are compiled separately

ifeq ($(dot-config),1)
# Read in config
-include include/config/auto.conf

# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
-include include/config/auto.conf.cmd

# To avoid any implicit rule to kick in, define an empty command
$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/auto.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
	$(Q)$(MAKE) -f $(srctree)/Makefile silentoldconfig

else
# Dummy target needed, because used as prerequisite
include/config/auto.conf: ;
endif # $(dot-config)

# The all: target is the default when no target is given on the
# command line.
# This allow a user to issue only 'make' to build the primary program
all: libcheck include/config/auto.conf $(PROGRAM)

# warn about C99 declaration after statement
KBUILD_CFLAGS += $(call cc-option,-Wdeclaration-after-statement,)

# disable pointer signed / unsigned warnings in gcc 4.0
KBUILD_CFLAGS += $(call cc-option,-Wno-pointer-sign,)

# disable invalid "can't wrap" optimizations for signed / pointers
KBUILD_CFLAGS	+= $(call cc-option,-fno-strict-overflow)

# revert to pre-gcc-4.4 behaviour of .eh_frame
KBUILD_CFLAGS	+= $(call cc-option,-fno-dwarf2-cfi-asm)

# conserve stack if available
KBUILD_CFLAGS   += $(call cc-option,-fconserve-stack)

# add extra debugging
ifdef CONFIG_DEBUG_INFO
KBUILD_CFLAGS	+= -g
KBUILD_AFLAGS	+= -gdwarf-2
endif

# Add user supplied CPPFLAGS, AFLAGS and CFLAGS as the last assignments
# But warn user when we do so
warn-assign = \
$(warning "WARNING: Appending $$K$(1) ($(K$(1))) from $(origin K$(1)) to kernel $$$(1)")

ifneq ($(KCPPFLAGS),)
        $(call warn-assign,CPPFLAGS)
        KBUILD_CPPFLAGS += $(KCPPFLAGS)
endif
ifneq ($(KAFLAGS),)
        $(call warn-assign,AFLAGS)
        KBUILD_AFLAGS += $(KAFLAGS)
endif
ifneq ($(KCFLAGS),)
        $(call warn-assign,CFLAGS)
        KBUILD_CFLAGS += $(KCFLAGS)
endif

# Use --build-id when available.
LDFLAGS_BUILD_ID = $(patsubst -Wl$(comma)%,%,\
			      $(call cc-ldoption, -Wl$(comma)--build-id,))
LDFLAGS_VMLINUX += $(LDFLAGS_BUILD_ID)

# Default kernel image to build when no specific target is given.
# KBUILD_IMAGE may be overruled on the command line or
# set in the environment
# Also any assignments in arch/$(ARCH)/Makefile take precedence over
# this default value
export KBUILD_IMAGE ?= vmlinux

#
# INSTALL_MOD_PATH specifies a prefix to MODLIB for module directory
# relocations required by build roots.  This is not defined in the
# makefile but the argument can be passed to make if needed.
#

MODLIB	= $(INSTALL_MOD_PATH)/lib/modules/$(KERNELRELEASE)
export MODLIB

#core-y		+= kernel/ mm/ fs/ ipc/ security/ crypto/ block/
#
#vmlinux-dirs	:= $(patsubst %/,%,$(filter %/, \
#		     $(core-y) $(core-m) $(drivers-y) $(drivers-m) \
#		     $(libs-y) $(libs-m)))
#
#vmlinux-alldirs	:= $(sort $(vmlinux-dirs) $(patsubst %/,%,$(filter %/, \
#		     $(init-n) $(init-) \
#		     $(core-n) $(core-) $(drivers-n) $(drivers-) \
#		     $(net-n)  $(net-)  $(libs-n)    $(libs-))))
#
#core-y		:= $(patsubst %/, %/built-in.o, $(core-y))
#drivers-y	:= $(patsubst %/, %/built-in.o, $(drivers-y))
#libs-y1		:= $(patsubst %/, %/lib.a, $(libs-y))
#libs-y2		:= $(patsubst %/, %/built-in.o, $(libs-y))
#libs-y		:= $(libs-y1) $(libs-y2)
core-y		+=

vmlinux-dirs	:= $(patsubst %/,%,$(filter %/, \
		     $(core-y) $(drivers-y) $(libs-y) ))

vmlinux-alldirs	:= $(sort $(vmlinux-dirs) $(patsubst %/,%,$(filter %/, \
		     $(core-n) $(core-) $(drivers-n) $(drivers-) \
		     $(libs-n)    $(libs-))))

core-y		:= $(patsubst %/, %/built-in.o, $(core-y))
drivers-y	:= $(patsubst %/, %/built-in.o, $(drivers-y))
libs-y1		:= $(patsubst %/, %/lib.a, $(libs-y))
libs-y2		:= $(patsubst %/, %/built-in.o, $(libs-y))
libs-y		:= $(libs-y1) $(libs-y2)

# Build vmlinux
# ---------------------------------------------------------------------------
# vmlinux is built from the objects selected by $(vmlinux-init) and
# $(vmlinux-main). Most are built-in.o files from top-level directories
# in the kernel tree, others are specified in arch/$(ARCH)/Makefile.
# Ordering when linking is important, and $(vmlinux-init) must be first.
#
# vmlinux
#   ^
#   |
#   +-< $(vmlinux-init)
#   |   +--< init/version.o + more
#   |
#   +--< $(vmlinux-main)
#   |    +--< driver/built-in.o mm/built-in.o + more
#   |
#   +-< kallsyms.o (see description in CONFIG_KALLSYMS section)
#
# vmlinux version (uname -v) cannot be updated during normal
# descending-into-subdirs phase since we do not yet know if we need to
# update vmlinux.
# Therefore this step is delayed until just before final link of vmlinux -
# except in the kallsyms case where it is done just before adding the
# symbols to the kernel.
#
# System.map is generated to document addresses of all kernel symbols

vmlinux-main := $(core-y) $(libs-y) $(drivers-y)
vmlinux-all  := $(vmlinux-main)
vmlinux-lds  :=
export KBUILD_VMLINUX_OBJS := $(vmlinux-all)

# Rule to link vmlinux - also used during CONFIG_KALLSYMS
# May be overridden by arch/$(ARCH)/Makefile
quiet_cmd_vmlinux__ ?= LD      $@
      cmd_vmlinux__ ?= $(LD) $(LDFLAGS) $(LDFLAGS_VMLINUX) -o $@ \
      -T $(vmlinux-lds) \
      --start-group $(vmlinux-main) --end-group                  \
      $(filter-out $(vmlinux-lds) $(vmlinux-main) vmlinux.o FORCE ,$^)

# Generate new vmlinux version
quiet_cmd_vmlinux_version = GEN     .version
      cmd_vmlinux_version = set -e;                     \
	if [ ! -r .version ]; then			\
	  rm -f .version;				\
	  echo 1 >.version;				\
	else						\
	  mv .version .old_version;			\
	  expr 0$$(cat .old_version) + 1 >.version;	\
	fi;						\
	$(MAKE) $(build)=init

# Generate System.map
quiet_cmd_sysmap = SYSMAP
      cmd_sysmap = $(CONFIG_SHELL) $(srctree)/scripts/mksysmap

# Link of vmlinux
# If CONFIG_KALLSYMS is set .version is already updated
# Generate System.map and verify that the content is consistent
# Use + in front of the vmlinux_version rule to silent warning with make -j2
# First command is ':' to allow us to use + in front of the rule
define rule_vmlinux__
	:
	$(call cmd,vmlinux__)
	$(Q)echo 'cmd_$@ := $(cmd_vmlinux__)' > $(@D)/.$(@F).cmd

	$(Q)$(if $($(quiet)cmd_sysmap),                                      \
	  echo '  $($(quiet)cmd_sysmap)  System.map' &&)                     \
	$(cmd_sysmap) $@ System.map;                                         \
	if [ $$? -ne 0 ]; then                                               \
		rm -f $@;                                                    \
		/bin/false;                                                  \
	fi;
endef

# The actual objects are generated when descending, 
# make sure no implicit rule kicks in
$(sort $(vmlinux-main)) $(vmlinux-lds): $(vmlinux-dirs) ;

# Handle descending into subdirectories listed in $(vmlinux-dirs)
# Preset locale variables to speed up the build process. Limit locale
# tweaks to this spot to avoid wrong language settings when running
# make menuconfig etc.
# Error messages still appears in the original language

PHONY += $(vmlinux-dirs)
$(vmlinux-dirs): prepare scripts
	$(Q)$(MAKE) $(build)=$@

CC_LDFLAGS = $(patsubst %,-Wl$(comma)%,$(LDFLAGS))

$(PROGRAM): $(vmlinux-all)
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(CC_LDFLAGS) $(MOSYS_MACROS) \
	$(LINUXINCLUDE) -o $@ $@.c $? $(LDLIBS) 

VPD_ENCODE_DEFCONFIG	:= "vpd_encode.config"
VPD_ENCODE_MACROS	:= -DPROGRAM=\"vpd_encode\" \
			   -DVERSION=\"$(RELEASENAME)\" \
			   -DVPD_ENCODE_CONFIG=\"$(VPD_ENCODE_DEFCONFIG)\"

# FIXME: should only depend on libs/ being a prerequisite
vpd_encode: $(core-y) $(libs-y)
	$(Q)$(CC) $(CFLAGS) $(VPD_ENCODE_MACROS) \
	$(EXTRA_CFLAGS) $(LDFLAGS) -Itools/vpd_encode $(LINUXINCLUDE) \
	-o $@ tools/vpd_encode/$@.c $? $(LDLIBS)
	$(Q)cp .config $(VPD_ENCODE_DEFCONFIG)
	$(Q)echo vpd_encode config file saved as "$(VPD_ENCODE_DEFCONFIG)"

TOOLS	+= vpd_encode

# Build the kernel release string
#
# The KERNELRELEASE value built here is stored in the file
# include/config/kernel.release, and is used when executing several
# make targets, such as "make install" or "make modules_install."
#
# The eventual kernel release string consists of the following fields,
# shown in a hierarchical format to show how smaller parts are concatenated
# to form the larger and final value, with values coming from places like
# the Makefile, kernel config options, make command line options and/or
# SCM tag information.
#
#	$(KERNELVERSION)
#	  $(VERSION)			eg, 2
#	  $(PATCHLEVEL)			eg, 6
#	  $(SUBLEVEL)			eg, 18
#	  $(EXTRAVERSION)		eg, -rc6
#	$(localver-full)
#	  $(localver)
#	    localversion*		(files without backups, containing '~')
#	    $(CONFIG_LOCALVERSION)	(from kernel config setting)
#	  $(localver-auto)		(only if CONFIG_LOCALVERSION_AUTO is set)
#	    ./scripts/setlocalversion	(SCM tag, if one exists)
#	    $(LOCALVERSION)		(from make command line if provided)
#
#  Note how the final $(localver-auto) string is included *only* if the
# kernel config option CONFIG_LOCALVERSION_AUTO is selected.  Also, at the
# moment, only git is supported but other SCMs can edit the script
# scripts/setlocalversion and add the appropriate checks as needed.

pattern = ".*/localversion[^~]*"
string  = $(shell cat /dev/null \
	   `find $(objtree) $(srctree) -maxdepth 1 -regex $(pattern) | sort -u`)

localver = $(subst $(space),, $(string) \
			      $(patsubst "%",%,$(CONFIG_LOCALVERSION)))

# If CONFIG_LOCALVERSION_AUTO is set scripts/setlocalversion is called
# and if the SCM is know a tag from the SCM is appended.
# The appended tag is determined by the SCM used.
#
# .scmversion is used when generating rpm packages so we do not loose
# the version information from the SCM when we do the build of the kernel
# from the copied source
ifdef CONFIG_LOCALVERSION_AUTO

ifeq ($(wildcard .scmversion),)
        _localver-auto = $(shell $(CONFIG_SHELL) \
                         $(srctree)/scripts/setlocalversion $(srctree))
else
        _localver-auto = $(shell cat .scmversion 2> /dev/null)
endif

	localver-auto  = $(LOCALVERSION)$(_localver-auto)
endif

localver-full = $(localver)$(localver-auto)

# Store (new) KERNELRELASE string in include/config/kernel.release
kernelrelease = $(KERNELVERSION)$(localver-full)
include/config/kernel.release: include/config/auto.conf FORCE
	$(Q)rm -f $@
	$(Q)echo $(kernelrelease) > $@


# Things we need to do before we recursively start building the kernel
# or the modules are listed in "prepare".
# A multi level approach is used. prepareN is processed before prepareN-1.
# version.h and scripts_basic is processed / created.

# Listed in dependency order
PHONY += prepare prepare0 prepare1 prepare2 prepare3

# prepare2 is used to check if we are building in a separate output directory,
# and if so do:
# 1) Check that make has not been executed in the kernel src $(srctree)
prepare2: include/config/kernel.release
ifneq ($(KBUILD_SRC),)
	@$(kecho) '  Using $(srctree) as source for kernel'
	$(Q)if [ -f $(srctree)/.config -o -d $(srctree)/include/config ]; then \
		echo "  $(srctree) is not clean, please run 'make mrproper'";\
		echo "  in the '$(srctree)' directory.";\
		/bin/false; \
	fi;
endif

prepare1: prepare2 include/linux/version.h \
                   include/config/auto.conf

prepare0: FORCE
	$(Q)$(MAKE) $(build)=.

# All the preparing..
prepare: prepare0

# Generate some files
# ---------------------------------------------------------------------------

# KERNELRELEASE can change from a few different places, meaning version.h
# needs to be updated, so this check is forced on all builds

uts_len := 64

define filechk_version.h
	(echo \#define LINUX_VERSION_CODE $(shell                             \
	expr $(VERSION) \* 65536 + $(PATCHLEVEL) \* 256 + $(SUBLEVEL));     \
	echo '#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))';)
endef

include/linux/version.h: $(srctree)/Makefile FORCE
	$(call filechk,version.h)

# make clean     Delete most generated files
#                Leave enough to build external modules

# Directories & files removed with 'make clean'
CLEAN_DIRS  += $(MODVERDIR)
CLEAN_FILES += $(PROGRAM) $(TESTPROGRAM) $(TOOLS)

# clean - Delete most, but leave enough to build external modules
#
clean: rm-dirs  := $(CLEAN_DIRS)
clean: rm-files := $(CLEAN_FILES)
clean-dirs      := $(addprefix _clean_,$(srctree) $(vmlinux-alldirs))

PHONY += $(clean-dirs) clean
$(clean-dirs):
	$(Q)$(MAKE) $(clean)=$(patsubst _clean_%,%,$@)

# Note: Cmockery uses an html file for documentation, so we need
# to be careful not to delete it along with lcov generated files.
lcov-clean:
	@find . $(RCS_FIND_IGNORE) $(CMOCKERY_FIND_IGNORE) \
		\( -name '*.css' -o -name '*.gcda' -o -name '*.png' \
		-o -name '*.css' -o -name '*.info' -o -name '*.html' \) \
		-type f -print | xargs rm -f
	@rm -rf $(GENHTML_OUTPUT_DIR)/

cmockery-clean:
	$(shell if [ -e $(CMOCKERY_PATH)/Makefile ]; then \
	                make -C $(CMOCKERY_PATH) clean >/dev/null; \
	        fi)

clean: $(clean-dirs) lcov-clean cmockery-clean
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)
	@find . $(RCS_FIND_IGNORE) \
		\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name '*.symtypes' -o -name 'modules.order' \
		-o -name modules.builtin -o -name '.tmp_*.o.*' \
		-o -name '*.gcno' \) -type f -print | xargs rm -f

# Brief documentation of the typical targets used
# ---------------------------------------------------------------------------

boards := $(wildcard $(srctree)/arch/$(SRCARCH)/configs/*_defconfig)
boards := $(notdir $(boards))
board-dirs := $(dir $(wildcard $(srctree)/arch/$(SRCARCH)/configs/*/*_defconfig))
board-dirs := $(sort $(notdir $(board-dirs:/=)))

help:
	@echo  'Cleaning targets:'
	@echo  '  clean		  - Remove most generated files but keep the config and'
	@echo  '                    enough build support to build external modules'
	@echo  ''
	@echo  'Configuration targets:'
	@$(MAKE) -f $(srctree)/scripts/kconfig/Makefile help
	@echo  ''
	@echo  'Other generic targets:'
	@echo  '  all		  - Build all targets marked with [*]'
	@echo  '  install	  - Install $(PROGRAM) to $(INSTALL_PATH)'
	@echo  '  export	  - Copy source code without VCS metadata'
	@echo  '  dir/            - Build all files in dir and below'
	@echo  '  dir/file.[ois]  - Build specified target only'
	@echo  '  kernelrelease	  - Output the release version string'
	@echo  '  kernelversion	  - Output the version stored in Makefile'
	@echo  ''
	@echo  ''
	@echo  'Architecture specific targets ($(SRCARCH)):'
	@$(if $(archhelp),$(archhelp),\
		echo '  No architecture specific help defined for $(SRCARCH)')
	@echo  ''
	@$(if $(boards), \
		$(foreach b, $(boards), \
		printf "  %-24s - Build for %s\\n" $(b) $(subst _defconfig,,$(b));) \
		echo '')
	@$(if $(board-dirs), \
		$(foreach b, $(board-dirs), \
		printf "  %-16s - Show %s-specific targets\\n" help-$(b) $(b);) \
		printf "  %-16s - Show all of the above\\n" help-boards; \
		echo '')

	@echo  '  make V=0|1 [targets] 0 => quiet build (default), 1 => verbose build'
	@echo  '  make V=2   [targets] 2 => give reason for rebuild of target'
	@echo  '  make O=dir [targets] Locate all output files in "dir", including .config'
	@echo  '  make C=1   [targets] Check all c source with $$CHECK (sparse by default)'
	@echo  '  make C=2   [targets] Force check of all c source with $$CHECK'
	@echo  ''
	@echo  'Execute "make" or "make all" to build all targets marked with [*] '
	@echo  'For further info see the ./README file'


help-board-dirs := $(addprefix help-,$(board-dirs))

help-boards: $(help-board-dirs)

boards-per-dir = $(notdir $(wildcard $(srctree)/arch/$(SRCARCH)/configs/$*/*_defconfig))

$(help-board-dirs): help-%:
	@echo  'Architecture specific targets ($(SRCARCH) $*):'
	@$(if $(boards-per-dir), \
		$(foreach b, $(boards-per-dir), \
		printf "  %-24s - Build for %s\\n" $*/$(b) $(subst _defconfig,,$(b));) \
		echo '')

# Scripts to check various things for consistency
# ---------------------------------------------------------------------------

endif #ifeq ($(config-targets),1)
endif #ifeq ($(mixed-targets),1)

PHONY += kernelrelease kernelversion

kernelrelease:
	$(if $(wildcard include/config/kernel.release), $(Q)echo $(KERNELRELEASE), \
	$(error kernelrelease not valid - run 'make prepare' to update it))
kernelversion:
	@echo $(KERNELVERSION)

# Single targets
# ---------------------------------------------------------------------------
# Single targets are compatible with:
# - build with mixed source and output
# - build with separate output dir 'make O=...'
# - external modules
#
#  target-dir => where to store outputfile
#  build-dir  => directory in kernel source tree to use

build-dir  = $(patsubst %/,%,$(dir $@))
target-dir = $(dir $@)

%.s: %.c prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.i: %.c prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.o: %.c prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.lst: %.c prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.s: %.S prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.o: %.S prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)
%.symtypes: %.c prepare scripts FORCE
	$(Q)$(MAKE) $(build)=$(build-dir) $(target-dir)$(notdir $@)

# FIXME Should go into a make.lib or something 
# ===========================================================================

quiet_cmd_rmdirs = $(if $(wildcard $(rm-dirs)),CLEAN   $(wildcard $(rm-dirs)))
      cmd_rmdirs = rm -rf $(rm-dirs)

quiet_cmd_rmfiles = $(if $(wildcard $(rm-files)),CLEAN   $(wildcard $(rm-files)))
      cmd_rmfiles = rm -f $(rm-files)

# Run depmod only if we have System.map and depmod is executable
quiet_cmd_depmod = DEPMOD  $(KERNELRELEASE)
      cmd_depmod = \
	if [ -r System.map -a -x $(DEPMOD) ]; then                              \
		$(DEPMOD) -ae -F System.map                                     \
		$(if $(strip $(INSTALL_MOD_PATH)), -b $(INSTALL_MOD_PATH) )     \
		$(KERNELRELEASE);                                               \
	fi

a_flags = -Wp,-MD,$(depfile) $(KBUILD_AFLAGS) $(AFLAGS_KERNEL) \
	  $(LINUXINCLUDE) $(KBUILD_CPPFLAGS) \
	  $(modkern_aflags) $(EXTRA_AFLAGS) $(AFLAGS_$(basetarget).o)

quiet_cmd_as_o_S = AS      $@
cmd_as_o_S       = $(CC) $(a_flags) -c -o $@ $<

# read all saved command lines

targets := $(wildcard $(sort $(targets)))
cmd_files := $(wildcard .*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  $(cmd_files): ;	# Do not try to update included dependency files
  include $(cmd_files)
endif

# Shorthand for $(Q)$(MAKE) -f scripts/Makefile.clean obj=dir
# Usage:
# $(Q)$(MAKE) $(clean)=dir
clean := -f $(if $(KBUILD_SRC),$(srctree)/)scripts/Makefile.clean obj

endif	# skip-makefile

PHONY += FORCE
FORCE:

PHONY += install
install: $(PROGRAM)
	mkdir -p $(INSTALL_PATH)
#	mkdir -p $(MANDIR)/man8
	$(INSTALL) -m 0755 $(PROGRAM) $(INSTALL_PATH)
#	$(INSTALL) -m 0644 $(PROGRAM).8 $(MANDIR)/man8

PHONY += export
export:
	@rm -rf $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)
	@svn export -r BASE . $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)
	@sed "s/^SVNVERSION.*/SVNVERSION := $(SVNVERSION)/" Makefile >$(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)/Makefile
	@LC_ALL=C svn log >$(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)/ChangeLog
	@echo Exported $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)/

PHONY += tarball
# TAROPTIONS reduces information leakage from the packager's system.
# If other tar programs support command line arguments for setting uid/gid of
# stored files, they can be handled here as well.
TAROPTIONS = $(shell LC_ALL=C tar --version|grep -q GNU && echo "--owner=root --group=root")
tarball: export
	@tar cjf $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME).tar.bz2 -C $(EXPORTDIR)/ $(TAROPTIONS) $(PROGRAM)-$(RELEASENAME)/
	@rm -rf $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME)
	@echo Created $(EXPORTDIR)/$(PROGRAM)-$(RELEASENAME).tar.bz2

libcmockery.a:
	@echo "Configuring cmockery..."
	$(Q)cd $(CMOCKERY_PATH) && ./configure >/dev/null && cd -
	@echo Building cmockery.
	$(Q)make --quiet -C $(CMOCKERY_PATH)
	ar rcs $@ $(CMOCKERY_PATH)/libcmockery_la-cmockery.o

PHONY += test
test: UNITTEST=y
test: CFLAGS += $(CFLAGS_GCOV)
test: KBUILD_CFLAGS+= $(CFLAGS_GCOV)
test: LINUXINCLUDE += $(CMOCKERY_INCLUDE)
test: MOSYS_MACROS += -DUNITTEST_DATA=\"$(UNITTEST_DATA)\"
test: $(vmlinux-all) libcmockery.a
	lcov --directory . --zerocounters
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(LDLIBS) $(CFLAGS_GCOV) \
	$(MOSYS_MACROS) $(LINUXINCLUDE) -o $(TESTPROGRAM) $(TESTPROGRAM).c $?
	@echo "Running $(TESTPROGRAM)"
	./$(TESTPROGRAM)
	lcov -b $(shell pwd) --directory . --capture \
	--output-file $(TESTPROGRAM).info --test-name $(TESTPROGRAM)
	genhtml -o $(GENHTML_OUTPUT_DIR)/ $(TESTPROGRAM).info
	@echo "Check results in $(GENHTML_OUTPUT_DIR)/index.html"

define LIBUUID_TEST
#include <uuid/uuid.h>
int main(void)
{
	uuid_t uuid;
	uuid_parse("88888888-4444-4444-4444-121212121212", uuid);
	return 0;
}
endef
export LIBUUID_TEST

test_libuuid:
	@echo "Testing libuuid..."
	@echo "$$LIBUUID_TEST" > .uuid_test.c
	@$(CC) $(CFLAGS) $(CC_LDFLAGS) -o .uuid_test .uuid_test.c -luuid >/dev/null 2>&1 && \
	echo "libuuid test passed." || \
	( echo "libuuid test failed. Please install libuuid" ; exit 1)
	@rm -f .uuid_test.c .uuid_test

define LIBFMAP_TEST
#include <inttypes.h>
#include <fmap.h>
int main(void)
{
	struct fmap *fmap = fmap_create(0, 0, (uint8_t *)"test");
	fmap_destroy(fmap);
	return 0;
}
endef
export LIBFMAP_TEST

test_libfmap:
	@echo "Testing libfmap..."
	@echo "$$LIBFMAP_TEST" > .fmap_test.c
	@$(CC) $(CFLAGS) $(CC_LDFLAGS) -o .fmap_test .fmap_test.c $(FMAP_LINKOPT) >/dev/null 2>&1 && \
	echo "libfmap test passed." || \
	( echo "libfmap test failed. Please install libfmap (http://flashmap.googlecode.com)"; exit 1 )
	@rm -f .fmap_test.c .fmap_test

libcheck: test_libuuid test_libfmap

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.

.PHONY: $(PHONY)
