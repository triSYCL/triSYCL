# Where the documentation is generated
DOXYGEN_DIR=tmp/Doxygen/SYCL

# The target to generate the documentation
doc: doc-API

# Generate the documenttion of the SYCL API
doc-API:
	mkdir -p $(DOXYGEN_DIR)
	doxygen doc/SYCL-API.doxygen
	# Compile the PDF version
	cd $(DOXYGEN_DIR)/latex; make

doc-clean:
	rm -rf $(DOXYGEN_DIR)
