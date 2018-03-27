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
  'src/libVasnecov/technologist.cpp',
  'src/libVasnecov/vasnecov.cpp',
  'src/libVasnecov/vasnecovelement.cpp',
  'src/libVasnecov/vasnecovfigure.cpp',
  'src/libVasnecov/vasnecovlabel.cpp',
  'src/libVasnecov/vasnecovlamp.cpp',
  'src/libVasnecov/vasnecovmaterial.cpp',
  'src/libVasnecov/vasnecovtexture.cpp',
  'src/libVasnecov/vasnecovuniverse.cpp',
  'src/libVasnecov/vasnecovworld.cpp',
  'src/libVasnecov/vasnecovmesh.cpp',
  'src/libVasnecov/vasnecovpipeline.cpp',
  'src/libVasnecov/vasnecovproduct.cpp',
  'src/libVasnecov/vasnecovscene.cpp',
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
)
