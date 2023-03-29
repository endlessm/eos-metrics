tests_env = environment()
tests_env.prepend('GI_TYPELIB_PATH', meson.project_build_root() / 'eosmetrics')

tests_c_args = [
  '-DG_LOG_DOMAIN="EosMetrics"',
  '-DCOMPILING_EOS_METRICS',
  '-D_POSIX_C_SOURCE=200112L',
]


###########################
# test-daemon-integration #
###########################

python = import('python')
python_exe = python.find_installation('python3')

test(
  'test-daemon-integration',
  python_exe,
  args: [
    files('test-daemon-integration.py'),
  ],
  workdir: meson.project_source_root(),
  env: tests_env,
)


#######################
# test-event-recorder #
#######################

test_event_recorder = executable(
  'test-event-recorder',
  files('test-event-recorder.c'),
  dependencies: [libeosmetrics_dep],
  c_args: tests_c_args,
)

test(
  'test-event-recorder',
  find_program('launch-mock-event-recorder-tests.sh'),
  args: [
    test_event_recorder,
    '--g-fatal-warnings',
    '--verbose',
    '-k',
  ],
  workdir: meson.project_source_root(),
  env: tests_env,
)


####################
# test-event-types #
####################

test_event_types = executable(
  'test-event-types',
  files('test-event-types.c'),
  dependencies: [libeosmetrics_dep],
  c_args: tests_c_args,
)
test('test-event-types', test_event_types, env: tests_env)
