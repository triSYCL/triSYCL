# Where the documentation is generated, to match also .doxygen files in
# doc/ and dev/publish_API
API_DOXYGEN_DIR=tmp/Doxygen/SYCL
IMPLEMENTATION_DOXYGEN_DIR=tmp/Doxygen/triSYCL
# To have a different documentation for ACAP++ instead of unified:
#IMPLEMENTATION_DOXYGEN_DIR=tmp/Doxygen/acappp

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
	# Compile the PDF version,
	# There are some bugs in Doxygen, so force the LaTeX compilation
	# even with errors by feeding LaTeX input with CR characters
	cd $(IMPLEMENTATION_DOXYGEN_DIR)/latex; yes | tr 'y' '\r' | make

doc-clean:
	rm -rf $(API_DOXYGEN_DIR) $(IMPLEMENTATION_DOXYGEN_DIR)


publish:
	./dev/publish_Doxygen
