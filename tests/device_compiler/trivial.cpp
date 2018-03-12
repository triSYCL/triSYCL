struct accessor {
  int gnih;

  accessor(int some) : gnih { some } { }

  void op() const { }
};

struct handler {

  template <typename ParallelForFunctor>
  void single_task(ParallelForFunctor &&f) {
    f();
  }

};

namespace drt {

struct machin {
  int bidule;

  machin(const accessor &other) : bidule { other.gnih } { }

  void op() const { }
};

}

int main(int argc, char *argv[]) {
  int a;
  handler h;
  auto b = accessor { a };

// before:
  h.single_task([=] { b.op(); });

// after:
  h.single_task([=, c = drt::machin { b }] { c.op(); });
}
