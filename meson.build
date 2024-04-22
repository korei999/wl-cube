project('wl-cube', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=2',
    'cpp_std=c++latest',
    'optimization=3',
    'buildtype=release',
    'debug=false'
    ])

add_project_arguments([
    '-DFPS_COUNTER'
    ], language: 'cpp')

if get_option('buildtype') == 'debug'
  add_project_arguments([
    '-DDEBUG', '-DLOGS', '-DFPS_COUNTER',
    # '-Wno-unused-parameter'
    ], language: 'cpp')
endif

gl_dep = dependency('gl')

executable('wl-cube',
           'controls.cc',
           'frame.cc',
           'gl.cc',
           'gmath.cc',
           'main.cc',
           'model.cc',
           'parser.cc',
           'shader.cc',
           'texture.cc',
           'utils.cc',
           'platform/windows/glad.cc',
           'platform/windows/windows.cc',
           'platform/windows/input.cc',
           dependencies : gl_dep,
           install : true)
