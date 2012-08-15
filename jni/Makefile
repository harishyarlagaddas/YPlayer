# Just Include the Makefiles of the corresponding libs. Everything like cleaning, building will be
# handled inside the module specific modules. You just include the correspondig Makefiles and just
# Enjoy....

.PHONY: default

.DEFAULT default:
	$(MAKE) -C ./os/build/linux/ $@
	$(MAKE) -C ./common/build/linux/ $@
	$(MAKE) -C ./external/build/linux/ $@
	$(MAKE) -C ./test/build/linux/ $@
