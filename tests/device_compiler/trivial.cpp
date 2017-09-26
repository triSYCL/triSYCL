struct accessor {
  accessor(int v) { }
};

namespace drt {
}

int main(int argc, char *argv[]) {
  int a;
  auto b = accessor { a };
}
