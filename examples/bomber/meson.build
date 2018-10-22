bomber_moc_headers = [
  'Scene.h',
]

bomber_src = [
  'bomber.cpp',
  'Scene.cpp',
]

bomber_processed = qt5_mod.preprocess(
  moc_headers : bomber_moc_headers,
)

bomber_exe = executable('bomber',
  sources : bomber_src + bomber_processed,
  link_with: [vasnecov_lib],
  dependencies : [vasnecov_dep, bmcl_dep, qt5_dep] + libs,
  cpp_args : cpp_args,
)
