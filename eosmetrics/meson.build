generated_files = emer_event_recorder_server_dbus_sources

version_config = configuration_data()
version_config.set('EMER_MAJOR_VERSION', major_version)
version_config.set('EMER_MINOR_VERSION', minor_version)
version_config.set('EMER_MICRO_VERSION', micro_version)

generated_files += configure_file(
  input: 'emtr-version.h.meson',
  output: 'emtr-version.h',
  configuration: version_config,
)

libeosmetrics_public_headers = files(
  'emtr-apiversion.h',
  'emtr-aggregate-timer.h',
  'emtr-enums.h',
  'emtr-event-recorder.h',
  'emtr-event-types.h',
  'emtr-macros.h',
  'emtr-types.h',
  'emtr-util.h',
)

libeosmetrics_sources = files(
  'emtr-aggregate-timer.c',
  'emtr-event-recorder.c',
  'emtr-event-types.c',
  'emtr-util.c',
)

libeosmetrics_all_sources = generated_files + libeosmetrics_sources


################
# Dependencies #
################

libeosmetrics_public_deps = [
  dependency('gobject-2.0'),
]

libeosmetrics_private_deps = [
  dependency('gio-unix-2.0', version: '>= 2.40'),
  dependency('uuid'),
]

libeosmetrics_deps = libeosmetrics_public_deps + libeosmetrics_private_deps


#################
# libeosmetrics #
#################

export_map_file = 'symbols.map'
libeosmetrics_library_name = package_name

libeosmetrics = shared_library(
  libeosmetrics_library_name,
  libeosmetrics_all_sources,
  version: library_version,
  gnu_symbol_visibility: 'default',
  link_args: [
    '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), export_map_file),
  ],
  link_depends: export_map_file,
  c_args: [
    '-DCOMPILING_EOS_METRICS',
    '-DG_LOG_DOMAIN="EosMetrics"',
    '-D_POSIX_C_SOURCE=200112L',
  ],
  include_directories: include_directories('..'), # FIXME: remove
  dependencies: libeosmetrics_deps,
  install: true,
)

install_headers(libeosmetrics_public_headers, install_dir: includedir / package_name)


#################
# Introspection #
#################

if get_option('introspection')
  libeosmetrics_gir = gnome.generate_gir(
    libeosmetrics,
    sources: libeosmetrics_sources + libeosmetrics_public_headers,
    nsversion: api_version,
    namespace: 'EosMetrics',
    symbol_prefix: 'emtr',
    identifier_prefix: 'Emtr',
    header: 'eosmetrics.h',
    export_packages: [package_name],
    includes: ['GLib-2.0', 'GObject-2.0', 'Gio-2.0'],
    link_with: libeosmetrics,
    install: true,
    install_dir_gir: datadir / 'gir-1.0',
    install_dir_typelib: datadir / 'girepository-1.0',
    fatal_warnings: true,
    extra_args: [
      '-DCOMPILING_EOS_METRICS',
      '--warn-all',
    ],
  )

  libeosmetrics_all_sources += [
    libeosmetrics_gir,
  ]
endif

libeosmetrics_dep = declare_dependency(
  link_with: libeosmetrics,
  include_directories: include_directories('..'), # FIXME: remove
  dependencies: libeosmetrics_deps,
  sources: libeosmetrics_all_sources,
)


##############
# pkg-config #
##############

pkgconfig.generate(
  libeosmetrics,
  description: 'Endless OS Metrics development kit',
  url: 'https://endlessos.org',
  subdirs: package_name,
  requires: libeosmetrics_public_deps,
  requires_private: libeosmetrics_private_deps,
  install_dir: libdir / 'pkgconfig',
)
