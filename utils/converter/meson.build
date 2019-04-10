cnv_moc_headers = [
  'ConverterWidget.h',
]

cnv_ui_files = [
  'ConverterWidget.ui',
]

cnv_src = [
  'ConverterWidget.cpp',
  'converter.cpp',
]

cnv_processed = qt5_mod.preprocess(
  ui_files : cnv_ui_files,
  moc_headers : cnv_moc_headers,
)

converter_exe = executable('converter',
  sources : cnv_src + cnv_processed,
  link_with: [vasnecov_lib],
  dependencies : [vasnecov_dep, bmcl_dep, qt5_dep] + libs,
  cpp_args : cpp_args,
)
