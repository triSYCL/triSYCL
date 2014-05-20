# The target to generate the documentation
doc: doc-API

# Generate the documenttion of the SYCL API
doc-API:

	doxygen doc/SYCL-API.doxygen
