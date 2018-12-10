project('vasnecov', 'cpp', default_options : ['cpp_std=c++11'])

cc = meson.get_compiler('cpp')

qt5_mod = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'OpenGL'])

bmcl_mod = subproject('bmcl', default_options: ['build_tests=false'])
bmcl_dep = bmcl_mod.get_variable('bmcl_dep')

rc_file = 'vasnecov.qrc'

inc = include_directories('src')

moc_headers = [
  'src/libVasnecov/vasnecovscene.h',
]

src = [
  'src/libVasnecov/configuration.h',
  'src/libVasnecov/coreobject.h',
  'src/libVasnecov/elementlist.h',
  'src/libVasnecov/technologist.cpp',
  'src/libVasnecov/technologist.h',
  'src/libVasnecov/types.h',
  'src/libVasnecov/vasnecov.cpp',
  'src/libVasnecov/vasnecov.h',
  'src/libVasnecov/vasnecovelement.cpp',
  'src/libVasnecov/vasnecovelement.h',
  'src/libVasnecov/vasnecovfigure.cpp',
  'src/libVasnecov/vasnecovfigure.h',
  'src/libVasnecov/vasnecovlabel.cpp',
  'src/libVasnecov/vasnecovlabel.h',
  'src/libVasnecov/vasnecovlamp.cpp',
  'src/libVasnecov/vasnecovlamp.h',
  'src/libVasnecov/vasnecovmaterial.cpp',
  'src/libVasnecov/vasnecovmaterial.h',
  'src/libVasnecov/vasnecovmesh.cpp',
  'src/libVasnecov/vasnecovmesh.h',
  'src/libVasnecov/vasnecovpipeline.cpp',
  'src/libVasnecov/vasnecovpipeline.h',
  'src/libVasnecov/vasnecovproduct.cpp',
  'src/libVasnecov/vasnecovproduct.h',
  'src/libVasnecov/vasnecovscene.cpp',
  'src/libVasnecov/vasnecovtexture.cpp',
  'src/libVasnecov/vasnecovtexture.h',
  'src/libVasnecov/vasnecovuniverse.cpp',
  'src/libVasnecov/vasnecovuniverse.h',
  'src/libVasnecov/vasnecovworld.cpp',
  'src/libVasnecov/vasnecovworld.h',
  'src/libVasnecov/version.h',
]

processed = qt5_mod.preprocess(
  include_directories : inc,
  qresources : rc_file,
  moc_headers : moc_headers,
)

cpp_args = []
if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  cpp_args = ['-Wall', '-Wextra', '-Wno-unused-parameter', '-Woverloaded-virtual', '-Weffc++']
endif

libs = []
if target_machine.system() == 'windows'
  libs += cc.find_library('OpenGL32', required : true)
  libs += cc.find_library('GLU32', required : true)
else
  libs += cc.find_library('GL', required : true)
  libs += cc.find_library('GLU', required : true)
endif

vasnecov_lib = static_library('vasnecov',
  name_prefix : 'lib',
  sources : src + processed,
  include_directories : inc,
  dependencies : [bmcl_dep, qt5_dep] + libs,
  cpp_args : cpp_args,
)

vasnecov_dep = declare_dependency(
  link_with : vasnecov_lib,
  include_directories : include_directories('src', is_system : true),
  dependencies : [bmcl_dep, qt5_dep],
)
