obj-y		+= command/

obj-y		+= alloc.o
obj-y		+= callbacks.o
obj-y		+= file_backed_range.o
obj-y		+= globals.o
obj-y		+= intf_list.o
obj-y		+= kv_pair.o
obj-y		+= list.o
obj-y		+= log.o
obj-y		+= output.o
obj-y		+= platform.o

# Big lock objects
obj-$(CONFIG_USE_IPC_LOCK)		+= big_lock.o
obj-$(CONFIG_USE_FILE_LOCK)		+= file_lock.o
obj-$(CONFIG_USE_SYSV_SEMAPHORE_LOCK)	+= ipc_lock.o
obj-$(CONFIG_USE_SYSV_SEMAPHORE_LOCK)	+= csem.o
