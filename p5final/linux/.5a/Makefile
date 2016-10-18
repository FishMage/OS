
INSTALL_PATH:=~cs537-1/ta/tests/.5a
PYTHON_PATH=python

all: runtests

runtests:
	echo "#!/bin/bash" > $@
	echo "base=$(INSTALL_PATH)" >> $@
	echo "$(PYTHON_PATH) \$$base/project5a.py --test-path \$$base \$$@"\
		" | tee runtests.log" >> $@
	echo "exit \$$?" >> $@
	chmod a+x runtests

install: runtests
	mkdir -p $(INSTALL_PATH)
	cp -r * $(INSTALL_PATH)
	afs_rseta $(INSTALL_PATH) system:anyuser read

clean:
	rm -f runtests*
	rm -f *.pyc
