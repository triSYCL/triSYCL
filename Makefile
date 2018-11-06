# Where the documentation is generated, to match also .doxygen files in
# doc/ and dev/publish_API
API_DOXYGEN_DIR=tmp/Doxygen/SYCL
IMPLEMENTATION_DOXYGEN_DIR=tmp/Doxygen/triSYCL

# The target to generate the documentation
#doc: doc-API doc-implementation
doc: doc-implementation

# Obsolete
# Generate the documentation of the SYCL API
#doc-API:
#	mkdir -p $(API_DOXYGEN_DIR)
#	doxygen doc/SYCL-API.doxygen
#	# Compile the PDF version
#	cd $(API_DOXYGEN_DIR)/latex; make

doc-implementation:
	mkdir -p $(IMPLEMENTATION_DOXYGEN_DIR)
	doxygen doc/triSYCL-implementation.doxygen
	# Compile the PDF version
	cd $(IMPLEMENTATION_DOXYGEN_DIR)/latex; make

doc-clean:
	rm -rf $(API_DOXYGEN_DIR) $(IMPLEMENTATION_DOXYGEN_DIR)


publish:
	./dev/publish_Doxygen
