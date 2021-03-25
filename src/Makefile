SUBDIRS := libfort openssl libevent libs dns_util libexternal \
	topo k8smgr

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKEFLAGS)

all: 
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir all; \
	done
new: 
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir new; \
	done
install: 
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir install; \
	done
clean: 
	@for dir in $(SUBDIRS); do \
	$(MAKE) -C $$dir clean; \
	done
distclean:
	rm -rf build
	mkdir -p build/include
	mkdir -p build/lib

.PHONY: subdirs $(SUBDIRS)
.PHONY: all new install clean
