project('eos-metrics',
  'c',
  version: '0.5.0',
  license: 'LGPL2+',
  meson_version: '>= 0.62'
)


gnome = import('gnome')
pkgconfig = import('pkgconfig')


##############
# Versioning #
##############

version = meson.project_version()
version_arr = version.split('.')
major_version = version_arr[0].to_int()
minor_version = version_arr[1].to_int()
micro_version = version_arr[2].to_int()
api_version = '0'
abi_version = '0'

library_version = '@0@.@1@.@2@'.format(abi_version, 100 * minor_version + micro_version, micro_version)

package_name = 'eosmetrics-@0@'.format(api_version)


#########
# Paths #
#########

prefix = get_option('prefix')
includedir = prefix / get_option('includedir')
libdir = prefix / get_option('libdir')
datadir = prefix / get_option('datadir')
localedir = prefix / get_option('localedir')


#########
# Build #
#########

subdir('data')
subdir('eosmetrics')

if get_option('docs')
  subdir('docs/reference/eosmetrics')
endif

if get_option('tests')
  subdir('tests')
endif


###########
# Summary #
###########

summary({
  'Documentation': get_option('docs'),
  'Introspection': get_option('introspection'),
  'Tests': get_option('tests'),
})
