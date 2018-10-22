df_moc_headers = [
  'Scene.h',
]

df_src = [
  'BomberModel.cpp',
  'ProductModel.cpp',
  'Scene.cpp',
  'dronesformation.cpp',
]

df_processed = qt5_mod.preprocess(
  moc_headers : df_moc_headers,
)

bomber_exe = executable('dronesformation',
  sources : df_src + df_processed,
  link_with: [vasnecov_lib],
  dependencies : [vasnecov_dep, bmcl_dep, qt5_dep] + libs,
  cpp_args : cpp_args,
)
