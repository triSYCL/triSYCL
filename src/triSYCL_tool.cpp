#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void
put_file_into_source(std::istream &input_file,
                     std::ostream &output_file) {
  output_file << R"(#include "trisycl.hpp"

namespace {

const char binary[] = {
)";

  for (;;) {
    char c;
    input_file.get(c);
    if (!input_file)
      break;
    output_file << "'\\x" << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<unsigned int>(static_cast<unsigned char>(c))
                << "', ";
  }
  output_file << R"(
};

trisycl::drt::code::program p { sizeof(binary), binary };

}
)";
}

int main(int argc, char *argv[]) {

  std::string output_file_name;

  // The description title when displaying the help
  po::options_description desc {
    "Some helper tools for triSYCL"
  };

  // Add the supported options
  desc.add_options()
    ("help,h", "produce this help message")
    ("source-in", po::value<std::string>(),
     R"(Take a file and put it into a C++ file to construct a kernel::code
 when compiled)")
    ("output,o", po::value<std::string>(&output_file_name),
     "specify the relay id (0--3)");

  // Where to get the option results
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  // Display the help if required
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("source-in")) {
    std::ifstream input_file { vm["source-in"].as<std::string>() };
    if (!input_file.is_open()) {
      std::cerr << "Failed to open \"" << vm["source-in"].as<std::string>()
                << "\" for --source-in" << std::endl;
      exit(-1);
    }
    input_file.exceptions(std::ifstream::badbit);
    if (output_file_name.empty())
      put_file_into_source(input_file, std::cout);
    else {
      std::ofstream output_file { output_file_name };
      if (!output_file.is_open()) {
        std::cerr << "Failed to open \"" << output_file_name
                  << "\" for --output" << std::endl;
        exit(-1);
      }
      output_file.exceptions(std::ifstream::badbit);
      put_file_into_source(input_file, output_file);
    }
    return 0;
  }

  std::cout << desc << std::endl;
  return 0;
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/
