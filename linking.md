# How To Link EzCellular

Options (from most to least recommended):

1. Use `pkgconfig` of installed version to automatically add the needed include and linking flags
  * **[RECOMMENDED]** meson: https://mesonbuild.com/Dependencies.html
  * cmake: https://cmake.org/cmake/help/latest/module/FindPkgConfig.html
  * manually: `g++ $(pkg-config --cflags ezcellular) example.cpp -o example $(pkg-config --libs ezcellular)`
2. Add Compiler flags manually (good luck :smile:)
3. Build this library manually (maybe as meson subproject) and link it statically
  * https://mesonbuild.com/Dependencies.html#building-dependencies-as-subprojects

An example Meson project definition `meson.build.example` is part of the example files.
Just rename it to `meson.build` and adjust contents as needed.
