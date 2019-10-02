# Where the documentation is generated, to match also .doxygen files in
# doc/ and dev/publish_API
API_DOXYGEN_DIR=tmp/Doxygen/SYCL
IMPLEMENTATION_DOXYGEN_DIR=tmp/Doxygen/acappp

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
	## Skip it for now because it does not compile :-(
	## pdfTeX error (ext4): \pdfendlink ended up in different nesting level than \pdfstartlink.
	## \AtBegShi@Output ...ipout \box \AtBeginShipoutBox \fi \fi
	## l.3846 ..._aie_ac19bae78f84462e7dfe1e85008f32744}}
	# cd $(IMPLEMENTATION_DOXYGEN_DIR)/latex; make

doc-clean:
	rm -rf $(API_DOXYGEN_DIR) $(IMPLEMENTATION_DOXYGEN_DIR)


publish:
	./dev/publish_Doxygen
