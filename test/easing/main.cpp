#include "easing/easing.hpp"
#include "interpolation/interpolation.hpp"
#include <boost/process.hpp>
#include <iostream>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

static void plot(const std::string &plot_name,
                 const std::string &points_data_path,
                 const std::string &output_png_path, bool is_dynamic) {
  boost::process::opstream write;
  boost::filesystem::path command("/usr/local/bin/gnuplot");
  boost::process::child plot(command, boost::process::std_in = write);

  write << "set terminal png\n";
  write << fmt::format("unset key\n");
  write << fmt::format("set output \"{}\"\n", output_png_path);
  write << fmt::format("set xlabel \"{}\"\n", "axis-x");
  write << fmt::format("set ylabel \"{}\"\n", "axis-y");
  write << fmt::format("set xrange {}\n", "[0:1]");
  write << fmt::format("set yrange {}\n", is_dynamic ? "[-0.5:1.5]" : "[0:1]");
  write << fmt::format("plot \"{}\" with points pt 7 lc 1,", points_data_path);

  write.flush();
  write.pipe().close();
  plot.wait();
  std::cout << plot_name << " was done, exit code: " << plot.exit_code()
            << std::endl;
}

template <typename F>
static void
plot(const std::string &plot_name, const std::string &points_data_path,
     const std::string &output_png_path, F easing, bool is_dynamic = false) {
  std::vector<float> xs(101), ys(101);
  for (int i = 0; i <= 100; i++) {
    xs[i] = i * 0.01f;
    ys[i] = interpolation::smoothstep(0.0f, 1.0f, easing(xs[i]));
  }
  std::ofstream points_file(points_data_path);
  for (int i = 0; i <= 100; i++) {
    points_file << xs[i] << " " << ys[i] << "\n";
  }
  points_file.close();

  plot(plot_name, points_data_path, output_png_path, is_dynamic);
}

template <typename T>
static void plot(const std::string &type_name, bool is_dynamic = false) {
  plot(fmt::format("{} in plot", type_name),
       fmt::format("data/{}_in.dat", type_name),
       fmt::format("data/{}_in.png", type_name), T::in, is_dynamic);
  plot(fmt::format("{} out plot", type_name),
       fmt::format("data/{}_out.dat", type_name),
       fmt::format("data/{}_out.png", type_name), T::out, is_dynamic);
  plot(fmt::format("{} in-out plot", type_name),
       fmt::format("data/{}_inout.dat", type_name),
       fmt::format("data/{}_inout.png", type_name), T::inout, is_dynamic);
}

int main() {
  plot("linear plot", "data/linear.dat", "data/linear.png",
       easing::linear<float>);
  plot("ease in plot", "data/ease_in.dat", "data/ease_in.png",
       easing::ease_in<float>);
  plot("ease out plot", "data/ease_out.dat", "data/ease_out.png",
       easing::ease_out<float>);
  plot("ease inout plot", "data/ease_inout.dat", "data/ease_inout.png",
       easing::ease_inout<float>);

  plot<easing::ease<easing::sine<float>, float>>("sine");
  plot<easing::ease<easing::quad<float>, float>>("quad");
  plot<easing::ease<easing::cubic<float>, float>>("cubic");
  plot<easing::ease<easing::quart<float>, float>>("quart");
  plot<easing::ease<easing::quint<float>, float>>("quint");
  plot<easing::ease<easing::expo<float>, float>>("expo");
  plot<easing::ease<easing::circ<float>, float>>("circ");
  plot<easing::ease<easing::back<float>, float>>("back", true);
  plot<easing::ease<easing::elastic<float>, float>>("elastic", true);
  plot<easing::ease<easing::bounce<float>, float>>("bounce");
  return 0;
}
