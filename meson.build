project('kindledopewars', 'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++17', 'warning_level=2'])

gtk_dep = dependency('gtk+-2.0')

executable('kindledopewars',
  'src/main.cpp',
  dependencies: [gtk_dep],
  install: false)
