gnome.gtkdoc(
  'eosmetrics',
  module_version: api_version,
  src_dir: meson.project_source_root() / 'eosmetrics',
  dependencies: libeosmetrics_dep,
  main_xml: 'eosmetrics-docs.xml',
  ignore_headers: [
    'emtr-aggregate-timer-private.h',
    'emtr-apiversion.h',
  ],
  scan_args: [
    '--deprecated-guards="EMTR_DISABLE_DEPRECATED"',
    '--rebuild-types',
  ],
  mkdb_args: [
    '--output-format=xml',
    '--xml-mode',
  ]
)
