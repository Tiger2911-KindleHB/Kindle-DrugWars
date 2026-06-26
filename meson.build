project('kindledopewars', 'cpp',
  version: '0.1.4',
  default_options: ['cpp_std=c++17', 'warning_level=2'])

gtk_dep = dependency('gtk+-2.0')
cpp = meson.get_compiler('cpp')
static_runtime_args = []
foreach arg : ['-static-libstdc++', '-static-libgcc']
  if cpp.has_link_argument(arg)
    static_runtime_args += arg
  endif
endforeach

executable('kindledopewars',
  'src/main.cpp',
  dependencies: [gtk_dep],
  link_args: static_runtime_args,
  install: false)
