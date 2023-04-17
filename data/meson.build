emer_event_recorder_server_dbus_sources = gnome.gdbus_codegen(
  'emer-event-recorder-server',
  'com.endlessm.Metrics.xml',
  interface_prefix: 'com.endlessm.Metrics.',
  autocleanup: 'all',
  namespace: 'Emer',
  install_header: true,
  install_dir: includedir / package_name,
)

install_data(
  'com.endlessm.Metrics.xml',
  install_dir: datadir / 'dbus-1' / 'interfaces',
)
