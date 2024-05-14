project('wl-cube', 'c', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=2',
    'cpp_std=c++23',
    'buildtype=release',
    'b_ndebug=if-release'
    ])

add_project_arguments(['-DFPS_COUNTER'], language: 'cpp')

if get_option('buildtype') == 'debug'
  add_project_arguments([
    '-DDEBUG', '-DLOGS',
    '-Wno-missing-field-initializers', '-Wno-unused-parameter'
    ], language: 'cpp')
endif

srcs = files('controls.cc',
             'frame.cc',
             'gl.cc',
             'gmath.cc',
             'main.cc',
             'model.cc',
             'parser.cc',
             'shader.cc',
             'texture.cc',
             'utils.cc')

deps = [dependency('gl')]

if build_machine.system() == 'linux'
  deps += [dependency('egl'),
           dependency('glesv2'),
           dependency('wayland-client'),
           dependency('wayland-egl'),
           dependency('wayland-cursor')]

  srcs += [files('platform/wayland/wayland.cc',
                 'platform/wayland/input.cc',
                 'platform/wayland/wayland-protocols/pointer-constraints-unstable-v1.c',
                 'platform/wayland/wayland-protocols/relative-pointer-unstable-v1.c',
                 'platform/wayland/wayland-protocols/xdg-shell.c')]
endif

if build_machine.system() == 'windows'
  srcs += [files('platform/windows/glad.cc',
                 'platform/windows/input.cc',
                 'platform/windows/windows.cc')]
endif



executable('wl-cube', sources : srcs, dependencies : deps, install : true)
