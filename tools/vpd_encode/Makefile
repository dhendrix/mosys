# Visit subdirectories first
obj-y					+= vendor_blobs/

# Note: vpd_encode.c contains main() and will be compiled at the end
# via the top-level Makefile
obj-$(CONFIG_BUILD_VPD)			+= lib_vpd_encode.o symbol.o
obj-y					+= type0.o
obj-y					+= type1.o
