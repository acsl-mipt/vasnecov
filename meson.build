project('vasnecov', 'cpp', default_options : ['cpp_std=c++11'])

cc = meson.get_compiler('cpp')

qt5_mod = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Widgets', 'OpenGL'])

bmcl_mod = subproject('bmcl', default_options: ['build_tests=false'])
bmcl_dep = bmcl_mod.get_variable('bmcl_dep')

rc_file = 'vasnecov.qrc'

inc = include_directories('src')

moc_headers = [
  'src/libVasnecov/VasnecovScene.h',
]

src = [
  'src/libVasnecov/Technologist.cpp',
  'src/libVasnecov/Vasnecov.cpp',
  'src/libVasnecov/VasnecovElement.cpp',
  'src/libVasnecov/VasnecovFigure.cpp',
  'src/libVasnecov/VasnecovLabel.cpp',
  'src/libVasnecov/VasnecovLamp.cpp',
  'src/libVasnecov/VasnecovMaterial.cpp',
  'src/libVasnecov/VasnecovMesh.cpp',
  'src/libVasnecov/VasnecovPipeline.cpp',
  'src/libVasnecov/VasnecovProduct.cpp',
  'src/libVasnecov/VasnecovResourceManager.cpp',
  'src/libVasnecov/VasnecovScene.cpp',
  'src/libVasnecov/VasnecovTerrain.cpp',
  'src/libVasnecov/VasnecovTexture.cpp',
  'src/libVasnecov/VasnecovUniverse.cpp',
  'src/libVasnecov/VasnecovWorld.cpp',
]

processed = qt5_mod.preprocess(
  include_directories : inc,
  qresources : rc_file,
  moc_headers : moc_headers,
)

cpp_args = []
if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  cpp_args = ['-Wall', '-Wextra', '-Wno-unused-parameter', '-Woverloaded-virtual']
endif

libs = []
if target_machine.system() == 'windows'
  libs += cc.find_library('OpenGL32', required : true)
  libs += cc.find_library('GLU32', required : true)
else
  libs += cc.find_library('GL', required : true)
  libs += cc.find_library('GLU', required : true)
endif

omp_dep = dependency('openmp', required : true)

vasnecov_lib = static_library('vasnecov',
  name_prefix : 'lib',
  sources : src + processed,
  include_directories : inc,
  dependencies : [bmcl_dep, qt5_dep, omp_dep] + libs,
  cpp_args : cpp_args,
)

vasnecov_dep = declare_dependency(
  link_with : vasnecov_lib,
  include_directories : include_directories('src', is_system : true),
  dependencies : [bmcl_dep, qt5_dep, omp_dep],
)

subdir('examples/bomber')
subdir('examples/dronesformation')
subdir('utils/converter')
