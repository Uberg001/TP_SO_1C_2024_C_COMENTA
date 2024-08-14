SUBDIRS := cpu entradasalida kernel memoria utils  # List of your subdirectories

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@