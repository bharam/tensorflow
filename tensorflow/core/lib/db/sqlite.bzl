"""Build support for SQLite extensions.

Using a SQL database for tensors poses challenges, sinces tensors aren't
exactly first-class data structures in these types of systems. So a lot
of the data we store is going to seem like opaque useless proprietary
BLOBs to folks who want to use their database the normal way.

The way we solve that is by writing native extensions for SQLite. We aim
to do it in a way that's agnostic of frameworks, platforms, libraries,
and tooling. The goal is for every user, regardless of his or her
personal preferences, to be able to do useful things with their data.

This Skylark module makes following those principles easy to do with
Bazel. It makes it possible to use our SQLite extension files both
statically and dynamically. In the dynamic case, the shared object is
made free of dependencies so it can be easily loaded into any
environment where SQLite exists, e.g. Python.
"""

load("//tensorflow:tensorflow.bzl", "tf_cc_shared_object")

def tf_sqlite_extension(name,
                        srcs,
                        hdrs=None,
                        data=None,
                        copts=[],
                        nocopts=None,
                        linkopts=[],
                        deps=[],
                        linkstatic=True,
                        linkshared=True,
                        visibility=None,
                        testonly=None,
                        licenses=None,
                        compatible_with=None,
                        restricted_to=None,
                        deprecation=None):
  """Defines a SQLite native extension.

  In essence, this is is cc_library(linkstatic) + cc_binary(linkshared).
  One can think of this macro as drop-in replacement for cc_library()
  that adapts Bazel's linkage idioms to how SQLite wants them to be.

  Args:
    name: String identifying the extension name. This needs to be valid
      as a symbol name
    srcs: List of C/C++ source files. One of these needs to be have a
      function named `sqlite3_NAME_init`. It also needs to have C
      linkage and be declared as a `SQLITE_CALLBACK`. Please note that
      when SQLite dynamically loads extensions, it infers the name of
      that symbol from the libNAME.so file.
    hdrs: Not allowed. When statically loading a SQLite extension, it's
      a much better idea to just put an ad-hoc extern declaration for
      your init function in a separate source file. Please note you can
      list .h files under srcs.
    data: List of files needed by the extension at runtime.
    copts: Extra compiler flags, passed along to the cc rules.
    nocopts: Standard Bazel attribute, passed along to cc rules.
    linkopts: Extra linker flags, passed along to the cc rules.
    deps: List of C/C++ dependencies, which should include the SQLite
      library, as it's not implicitly added by this macro.
    linkstatic: Can be used to disable static linking support.
    linkshared: Can be used to disable dynamic linking support.
    visibility: Standard Bazel attribute. Please note intermediate files
      created by this macro will always have private visibility.
    testonly: Standard Bazel attribute.
    licenses: Standard Bazel attribute.
    compatible_with: Standard Bazel attribute.
    restricted_to: Standard Bazel attribute.
    deprecation: Standard Bazel attribute.
  """
  if hdrs:
    fail("SQLite extensions should list .h files under srcs")
  if not linkstatic and not linkshared:
    fail("Must specify linkstatic or linkshared")

  if linkstatic:
    native.cc_library(
        name = name,
        srcs = srcs,
        data = data,
        copts = copts + ["-DSQLITE_OMIT_LOAD_EXTENSION"],
        linkopts = linkopts,
        nocopts = nocopts,
        deps = deps,
        linkstatic = 1,
        licenses = licenses,
        visibility = visibility,
        testonly = testonly,
        deprecation = deprecation,
        compatible_with = compatible_with,
        restricted_to = restricted_to,
    )

  if not linkshared:
    return

  p = name.rfind("/")
  if p == -1:
    sname = name
    prefix = ""
  else:
    sname = name[p + 1:]
    prefix = name[:p + 1]

  so_file = "%slib%s.so" % (prefix, sname)
  symbol = "sqlite3_%s_init" % sname
  exported_symbols_file = "%s-exported-symbols.lds" % name
  version_script_file = "%s-version-script.lds" % name

  native.genrule(
      name = name + "_exported_symbols",
      outs = [exported_symbols_file],
      cmd = "echo %s >$@" % symbol,
      output_licenses = ["unencumbered"],
      visibility = ["//visibility:private"],
      testonly = testonly,
  )

  native.genrule(
      name = name + "_version_script",
      outs = [version_script_file],
      cmd = "echo '{global: %s; local: *;};' >$@" % symbol,
      output_licenses = ["unencumbered"],
      visibility = ["//visibility:private"],
      testonly = testonly,
  )

  native.cc_library(
      name = name + "_dynamic",
      srcs = srcs,
      data = data,
      copts = copts + select({
          "//tensorflow:windows_msvc": [
              "-DSQLITE_CALLBACK=__declspec(dllexport)",
          ],
          "//conditions:default": [],
      }),
      nocopts = nocopts,
      deps = deps,
      licenses = licenses,
      testonly = testonly,
      compatible_with = compatible_with,
      restricted_to = restricted_to,
      visibility = ["//visibility:private"],
      linkstatic = 1,
  )

  tf_cc_shared_object(
      name = so_file,
      framework_so = [],
      linkopts = linkopts + select({
          "//tensorflow:darwin": [
              "-Wl,-exported_symbols_list",
              exported_symbols_file,
          ],
          "//tensorflow:windows": [],
          "//tensorflow:windows_msvc": [],
          "//conditions:default": [
              "-z defs",
              "-s",
              "-Wl,--version-script",
              version_script_file,
          ],
      }),
      deps = [
          exported_symbols_file,
          version_script_file,
          ":%s_dynamic" % name,
      ],
      visibility = visibility,
      testonly = testonly,
      deprecation = deprecation,
      compatible_with = compatible_with,
      restricted_to = restricted_to,
  )

  if not linkstatic:
    native.alias(
        name = name,
        actual = so_file,
        testonly = testonly,
        visibility = visibility,
    )
