# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/vadim/Projects/appCreatorShader/src/libs/imgui"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-build"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/tmp"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/src/imgui_fc-populate-stamp"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/src"
  "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/src/imgui_fc-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/src/imgui_fc-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/vadim/Projects/appCreatorShader/build-sys/_deps/imgui-subbuild/imgui_fc-populate-prefix/src/imgui_fc-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
