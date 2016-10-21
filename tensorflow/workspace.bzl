# TensorFlow external dependencies that can be loaded in WORKSPACE files.

load("//third_party/gpus:cuda_configure.bzl", "cuda_configure")

# If TensorFlow is linked as a submodule.
# path_prefix and tf_repo_name are no longer used.
def tf_workspace(path_prefix = "", tf_repo_name = ""):
  cuda_configure(name = "local_config_cuda")
  if path_prefix:
    print("path_prefix was specified to tf_workspace but is no longer used and will be removed in the future.")
  if tf_repo_name:
    print("tf_repo_name was specified to tf_workspace but is no longer used and will be removed in the future.")

  # These lines need to be changed when updating Eigen. They are parsed from
  # this file by the cmake and make builds to determine the eigen version and
  # hash.
  eigen_version = "c78d757b69d3"
  eigen_sha256 = "dfb650e20a0dee6172dcc99796210a07e40af61348497503b42dc12935b4e6f5"

  native.new_http_archive(
    name = "eigen_archive",
    url = "http://bitbucket.org/eigen/eigen/get/" + eigen_version + ".tar.gz",
    sha256 = eigen_sha256,
    strip_prefix = "eigen-eigen-" + eigen_version,
    build_file = str(Label("//:eigen.BUILD")),
  )

  native.http_archive(
    name = "com_googlesource_code_re2",
    url = "http://github.com/google/re2/archive/7bab3dc83df6a838cc004cc7a7f51d5fe1a427d5.tar.gz",
    sha256 = "ef91af8850f734c8be65f2774747f4c2d8d81e556ba009faa79b4dd8b2759555",
    strip_prefix = "re2-7bab3dc83df6a838cc004cc7a7f51d5fe1a427d5",
  )

  native.http_archive(
    name = "gemmlowp",
    url = "http://github.com/google/gemmlowp/archive/8b20dd2ce142115857220bd6a35e8a081b3e0829.tar.gz",
    sha256 = "9cf5f1e3d64b3632dbae5c65efb79f4374ca9ac362d788fc61e086af937ff6d7",
    strip_prefix = "gemmlowp-8b20dd2ce142115857220bd6a35e8a081b3e0829",
  )

  native.new_http_archive(
    name = "farmhash_archive",
    url = "http://github.com/google/farmhash/archive/71a777924015693c69bc3c8c6492fb8d5372c636.zip",
    sha256 = "99190108fb96a5e38e183f6a23fb7742948214fc96a746a50c79eb09a255a298",
    strip_prefix = "farmhash-71a777924015693c69bc3c8c6492fb8d5372c636/src",
    build_file = str(Label("//:farmhash.BUILD")),
  )

  native.bind(
    name = "farmhash",
    actual = "@farmhash//:farmhash",
  )

  native.http_archive(
    name = "highwayhash",
    url = "http://github.com/google/highwayhash/archive/4bce8fc6a9ca454d9d377dbc4c4d33488bbab78f.tar.gz",
    sha256 = "b159a62fb05e5f6a6be20aa0df6a951ebf44a7bb96ed2e819e4e35e17f56854d",
    strip_prefix = "highwayhash-4bce8fc6a9ca454d9d377dbc4c4d33488bbab78f",
  )

  native.new_http_archive(
    name = "jpeg_archive",
    url = "http://www.ijg.org/files/jpegsrc.v9a.tar.gz",
    sha256 = "3a753ea48d917945dd54a2d97de388aa06ca2eb1066cbfdc6652036349fe05a7",
    strip_prefix = "jpeg-9a",
    build_file = str(Label("//:jpeg.BUILD")),
  )

  native.new_http_archive(
    name = "png_archive",
    url = "http://github.com/glennrp/libpng/archive/v1.2.53.zip",
    sha256 = "c35bcc6387495ee6e757507a68ba036d38ad05b415c2553b3debe2a57647a692",
    strip_prefix = "libpng-1.2.53",
    build_file = str(Label("//:png.BUILD")),
  )

  native.new_http_archive(
    name = "gif_archive",
    url = "http://ufpr.dl.sourceforge.net/project/giflib/giflib-5.1.4.tar.gz",
    sha256 = "34a7377ba834397db019e8eb122e551a49c98f49df75ec3fcc92b9a794a4f6d1",
    strip_prefix = "giflib-5.1.4/lib",
    build_file = str(Label("//:gif.BUILD")),
  )

  native.new_http_archive(
    name = "six_archive",
    url = "http://pypi.python.org/packages/source/s/six/six-1.10.0.tar.gz",
    sha256 = "105f8d68616f8248e24bf0e9372ef04d3cc10104f1980f54d57b2ce73a5ad56a",
    strip_prefix = "six-1.10.0",
    build_file = str(Label("//:six.BUILD")),
  )

  native.bind(
    name = "six",
    actual = "@six_archive//:six",
  )

  native.http_archive(
    name = "protobuf",
    url = "http://github.com/google/protobuf/archive/v3.1.0.tar.gz",
    sha256 = "0a0ae63cbffc274efb573bdde9a253e3f32e458c41261df51c5dbc5ad541e8f7",
    strip_prefix = "protobuf-3.1.0",
  )

  native.new_http_archive(
    name = "gmock_archive",
    url = "http://pkgs.fedoraproject.org/repo/pkgs/gmock/gmock-1.7.0.zip/073b984d8798ea1594f5e44d85b20d66/gmock-1.7.0.zip",
    sha256 = "26fcbb5925b74ad5fc8c26b0495dfc96353f4d553492eb97e85a8a6d2f43095b",
    strip_prefix = "gmock-1.7.0",
    build_file = str(Label("//:gmock.BUILD")),
  )

  native.bind(
    name = "gtest",
    actual = "@gmock_archive//:gtest",
  )

  native.bind(
    name = "gtest_main",
    actual = "@gmock_archive//:gtest_main",
  )

  native.bind(
    name = "python_headers",
    actual = str(Label("//util/python:python_headers")),
  )

  # grpc expects //external:protobuf_clib and //external:protobuf_compiler
  # to point to the protobuf's compiler library.
  native.bind(
    name = "protobuf_clib",
    actual = "@protobuf//:protoc_lib",
  )

  native.bind(
    name = "protobuf_compiler",
    actual = "@protobuf//:protoc_lib",
  )

  native.new_http_archive(
    name = "grpc",
    url = "http://github.com/grpc/grpc/archive/d7ff4ff40071d2b486a052183e3e9f9382afb745.tar.gz",
    sha256 = "a15f352436ab92c521b1ac11e729e155ace38d0856380cf25048c5d1d9ba8e31",
    strip_prefix = "grpc-d7ff4ff40071d2b486a052183e3e9f9382afb745",
    build_file = str(Label("//:grpc.BUILD")),
  )

  # protobuf expects //external:grpc_cpp_plugin to point to grpc's
  # C++ plugin code generator.
  native.bind(
    name = "grpc_cpp_plugin",
    actual = "@grpc//:grpc_cpp_plugin",
  )

  native.bind(
    name = "grpc_lib",
    actual = "@grpc//:grpc++_unsecure",
  )

  native.new_git_repository(
    name = "linenoise",
    commit = "c894b9e59f02203dbe4e2be657572cf88c4230c3",
    init_submodules = True,
    remote = "https://github.com/antirez/linenoise.git",
    build_file = str(Label("//:linenoise.BUILD")),
  )

  native.new_http_archive(
    name = "jsoncpp_git",
    url = "http://github.com/open-source-parsers/jsoncpp/archive/11086dd6a7eba04289944367ca82cea71299ed70.tar.gz",
    sha256 = "07d34db40593d257324ec5fb9debc4dc33f29f8fb44e33a2eeb35503e61d0fe2",
    strip_prefix = "jsoncpp-11086dd6a7eba04289944367ca82cea71299ed70",
    build_file = str(Label("//:jsoncpp.BUILD")),
  )

  native.bind(
    name = "jsoncpp",
    actual = "@jsoncpp_git//:jsoncpp",
  )

  native.http_archive(
    name = "boringssl",
    url = "http://github.com/google/boringssl/archive/bbcaa15b0647816b9a1a9b9e0d209cd6712f0105.tar.gz",  # 2016-07-11
    sha256 = "025264d6e9a7ad371f2f66d17a28b6627de0c9592dc2eb54afd062f68f1f9aa3",
    strip_prefix = "boringssl-bbcaa15b0647816b9a1a9b9e0d209cd6712f0105",
  )

  native.new_http_archive(
    name = "nanopb_git",
    url = "http://github.com/nanopb/nanopb/archive/1251fa1065afc0d62f635e0f63fec8276e14e13c.tar.gz",
    sha256 = "ab1455c8edff855f4f55b68480991559e51c11e7dab060bbab7cffb12dd3af33",
    strip_prefix = "nanopb-1251fa1065afc0d62f635e0f63fec8276e14e13c",
    build_file = str(Label("//:nanopb.BUILD")),
  )

  native.bind(
    name = "nanopb",
    actual = "@nanopb_git//:nanopb",
  )

  native.new_http_archive(
    name = "zlib_archive",
    url = "http://zlib.net/zlib-1.2.8.tar.gz",
    sha256 = "36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d",
    strip_prefix = "zlib-1.2.8",
    build_file = str(Label("//:zlib.BUILD")),
  )

  native.bind(
    name = "zlib",
    actual = "@zlib_archive//:zlib",
  )

  ##############################################################################
  # TensorBoard Build Tools

  native.new_http_archive(
      name = "nodejs",
      build_file = str(Label("//third_party:nodejs.BUILD")),
      sha256 = "4350d0431b49697517c6cca5d66adf5f74eb9101c52f52ae959fa94225822d44",
      url = "http://nodejs.org/dist/v4.3.2/node-v4.3.2-linux-x64.tar.xz",
      strip_prefix = "node-v4.3.2-linux-x64",
      # avoid using node; realpath optimization doesn't jive with bazel
  )

  native.http_file(
      name = "tsc_js",
      sha256 = "bb785c75c147e050a77a0fff6a52a117a66f9c803ef9278fa49e7714b15f3217",
      url = "https://raw.githubusercontent.com/Microsoft/TypeScript/v2.0.5/lib/tsc.js",
  )

  ##############################################################################
  # TensorBoard JavaScript Production Dependencies

  native.http_file(
      name = "d3_js",
      sha256 = "bc1e38838f5c5c8e040132d41efee6bfddbef728210bd566479dc1694af1d3f5",
      url = "https://raw.githubusercontent.com/d3/d3/v3.5.15/d3.js",
      # no @license header
  )

  native.http_file(
      name = "dagre_core_js",
      sha256 = "7323829ddd77924a69e2b1235ded3eac30acd990da0f037e0fbd3c8e9035b50d",
      url = "https://raw.githubusercontent.com/cpettitt/dagre/v0.7.4/dist/dagre.core.js",
      # no @license header
  )

  native.http_file(
      name = "graphlib_core_js",
      sha256 = "772045d412b1513b549be991c2e1846c38019429d43974efcae943fbe83489bf",
      url = "https://raw.githubusercontent.com/cpettitt/graphlib/v1.0.7/dist/graphlib.core.js",
      # no @license header
  )

  native.http_file(
      name = "lodash_js",
      sha256 = "7c7b391810bc08cf815683431857c51b5ee190062ae4f557e1e4689d6dd910ea",
      url = "https://raw.githubusercontent.com/lodash/lodash/3.8.0/lodash.js",
  )

  native.http_file(
      name = "numeric_js",
      sha256 = "dfaca3b8485bee735788cc6eebca82ea25719adc1fb8911c7799c6bd5a95df3b",
      url = "https://raw.githubusercontent.com/jart/numeric/v1.2.6/src/numeric.js",
      # no @license header
  )

  native.http_file(
      name = "orbitcontrols_js",
      sha256 = "0e98ded15bb7fe398a655667e76b39909d36c0973a8950d01c62f65f93161c27",
      url = "https://raw.githubusercontent.com/mrdoob/three.js/ad419d40bdaab80abbb34b8f359b4ee840033a02/examples/js/controls/OrbitControls.js",
      # no @license header
  )

  native.http_file(
      name = "plottable_css",
      sha256 = "77510d7538dbd3b59f1c8a06f68131b38562e3be546364747618d5112723e818",
      url = "https://raw.githubusercontent.com/palantir/plottable/v1.16.1/plottable.css",
      # no @license header
  )

  native.http_file(
      name = "plottable_js",
      sha256 = "32647b0fb4175fa875a71e6d56c761b88d975186ed6a8820e2c7854165a8988d",
      url = "https://raw.githubusercontent.com/palantir/plottable/v1.16.1/plottable.js",
      # no @license header
  )

  native.http_file(
      name = "three_js",
      sha256 = "7aff264bd84c90bed3c72a4dc31db8c19151853c6df6980f52b01d3e9872c82d",
      url = "https://raw.githubusercontent.com/mrdoob/three.js/ad419d40bdaab80abbb34b8f359b4ee840033a02/build/three.js",
      # no @license header
  )

  native.new_http_archive(
      name = "webcomponentsjs",
      build_file = str(Label("//third_party:webcomponentsjs.BUILD")),
      url = "https://github.com/webcomponents/webcomponentsjs/archive/v0.7.22.tar.gz",
      strip_prefix = "webcomponentsjs-0.7.22",
  )

  native.http_file(
      name = "weblas_js",
      sha256 = "f138fce57f673ca8a633f4aee5ae5b6fcb6ad0de59069a42a74e996fd04d8fcc",
      url = "https://raw.githubusercontent.com/waylonflinn/weblas/v0.9.0/dist/weblas.js",
      # no @license header
  )

  ##############################################################################
  # TensorBoard TypeScript External Definitions Dependencies

  native.http_file(
      name = "d3_d_ts",
      sha256 = "177293828c7a206bf2a7f725753d51396d38668311aa37c96445f91bbf8128a7",
      url = "https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/6e2f2280ef16ef277049d0ce8583af167d586c59/d3/d3.d.ts",
  )

  native.http_file(
      name = "lib_es6_d_ts",
      sha256 = "f4de46e04293569a666f2045f850d90e16dc8ba059af02b5a062942245007a71",
      url = "https://raw.githubusercontent.com/Microsoft/TypeScript/v2.0.5/lib/lib.es6.d.ts",
  )

  native.http_file(
      name = "lodash_d_ts",
      sha256 = "e4cd3d5de0eb3bc7b1063b50d336764a0ac82a658b39b5cf90511f489ffdee60",
      url = "https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/efd40e67ff323f7147651bdbef03c03ead7b1675/lodash/lodash.d.ts",
  )

  native.http_file(
      name = "plottable_d_ts",
      sha256 = "cd46dc709b01cd361e8399f797760871a6a207bc832e08fcff385ced02ef2b43",
      url = "https://raw.githubusercontent.com/palantir/plottable/v1.16.1/plottable.d.ts",
  )

  ##############################################################################
  # TensorBoard JavaScript Testing Dependencies

  native.http_file(
      name = "chai_d_ts",
      sha256 = "b7da645f6e5555feb7aeede73775da0023ce2257df9c8e76c9159266035a9c0d",
      url = "https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/ebc69904eb78f94030d5d517b42db20867f679c0/chai/chai.d.ts",
  )

  native.http_file(
      name = "chai_js",
      sha256 = "b926b325ad9843bf0b7a6d580ef78bb560e47c484b98680098d4fd9b31b77cd9",
      url = "https://raw.githubusercontent.com/chaijs/chai/2.3.0/chai.js",
      # no @license header
  )

  native.http_file(
      name = "mocha_d_ts",
      sha256 = "695a03dd2ccb238161d97160b239ab841562710e5c4e42886aefd4ace2ce152e",
      url = "https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/ebc69904eb78f94030d5d517b42db20867f679c0/mocha/mocha.d.ts",
  )

  native.http_file(
      name = "mocha_js",
      sha256 = "e36d865a17ffdf5868e55e736526ae30f3d4bc667c85a2a28cd5c850a82361e2",
      url = "https://raw.githubusercontent.com/mochajs/mocha/2.3.4/mocha.js",
      # no @license header
  )

  ##############################################################################
  # TensorBoard JavaScript Demo Dependencies

  native.new_http_archive(
      name = "marked",
      build_file = str(Label("//third_party:marked.BUILD")),
      sha256 = "13bdba8819aa53f3ac694f77d79cc19f26a95b458749bc72c2c4fd0d29215bb6",
      url = "https://github.com/chjj/marked/archive/v0.3.5.zip",
      strip_prefix = "marked-0.3.5",
  )

  native.new_http_archive(
      name = "prism",
      build_file = str(Label("//third_party:prism.BUILD")),
      sha256 = "e06eb54f2a80e6b3cd0bd4d59f900423bcaee53fc03998a056df63740c684683",
      url = "https://github.com/PrismJS/prism/archive/abee2b7587f1925e57777044270e2a1860810994.tar.gz",
      strip_prefix = "prism-abee2b7587f1925e57777044270e2a1860810994",
  )

  ##############################################################################
  # TensorBoard Polymer Production Dependencies

  native.new_http_archive(
      name = "font_roboto",
      build_file = str(Label("//third_party:font_roboto.BUILD")),
      sha256 = "fae51429b56a4a4c15f1f0c23b733c7095940cc9c04c275fa7adb3bf055b23b3",
      url = "https://github.com/PolymerElements/font-roboto/archive/v1.0.1.tar.gz",
      strip_prefix = "font-roboto-1.0.1",
  )

  native.new_http_archive(
      name = "iron_a11y_announcer",
      build_file = str(Label("//third_party:iron_a11y_announcer.BUILD")),
      sha256 = "0c3f88152529aae2cad7ea9a4cc542852872d5268c56754ec8433767923f57bb",
      url = "https://github.com/PolymerElements/iron-a11y-announcer/archive/v1.0.4.tar.gz",
      strip_prefix = "iron-a11y-announcer-1.0.4",
  )

  native.new_http_archive(
      name = "iron_a11y_keys_behavior",
      build_file = str(Label("//third_party:iron_a11y_keys_behavior.BUILD")),
      sha256 = "8db35e51599f400b7c3dc888103610a2c9c8c0e114aa008b43f69352733701bd",
      url = "https://github.com/PolymerElements/iron-a11y-keys-behavior/archive/v1.1.2.tar.gz",
      strip_prefix = "iron-a11y-keys-behavior-1.1.2",
  )

  native.new_http_archive(
      name = "iron_ajax",
      build_file = str(Label("//third_party:iron_ajax.BUILD")),
      sha256 = "9162d8af4611e911ac3ebbfc08bb7038ac04f6e79a9287b1476fe36ad6770bc5",
      url = "https://github.com/PolymerElements/iron-ajax/archive/v1.2.0.tar.gz",
      strip_prefix = "iron-ajax-1.2.0",
  )

  native.new_http_archive(
      name = "iron_autogrow_textarea",
      build_file = str(Label("//third_party:iron_autogrow_textarea.BUILD")),
      sha256 = "50bbb901d2c8f87462e3552e3d671a552faa12c37c485e548d7a234ebffbc427",
      url = "https://github.com/PolymerElements/iron-autogrow-textarea/archive/v1.0.12.tar.gz",
      strip_prefix = "iron-autogrow-textarea-1.0.12",
  )

  native.new_http_archive(
      name = "iron_behaviors",
      build_file = str(Label("//third_party:iron_behaviors.BUILD")),
      sha256 = "a1e8d4b7a13f3d36beba9c2a6b186ed33a53e6af2e79f98c1fcc7e85e7b53f89",
      url = "https://github.com/PolymerElements/iron-behaviors/archive/v1.0.17.tar.gz",
      strip_prefix = "iron-behaviors-1.0.17",
  )

  native.new_http_archive(
      name = "iron_checked_element_behavior",
      build_file = str(Label("//third_party:iron_checked_element_behavior.BUILD")),
      sha256 = "539a0e1c4df0bc702d3bd342388e4e56c77ec4c2066cce69e41426a69f92e8bd",
      url = "https://github.com/PolymerElements/iron-checked-element-behavior/archive/v1.0.4.tar.gz",
      strip_prefix = "iron-checked-element-behavior-1.0.4",
  )

  native.new_http_archive(
      name = "iron_collapse",
      build_file = str(Label("//third_party:iron_collapse.BUILD")),
      sha256 = "275808994a609a2f9923e2dd2db1957945ab141ba840eadc33f19e1f406d600e",
      url = "https://github.com/PolymerElements/iron-collapse/archive/v1.0.8.tar.gz",
      strip_prefix = "iron-collapse-1.0.8",
  )

  native.new_http_archive(
      name = "iron_dropdown",
      build_file = str(Label("//third_party:iron_dropdown.BUILD")),
      sha256 = "f7e4a31d096d10d8af1920397695cb17f3eb1cbe5e5ff91a861dabfcc085f376",
      url = "https://github.com/PolymerElements/iron-dropdown/archive/v1.4.0.tar.gz",
      strip_prefix = "iron-dropdown-1.4.0",
  )

  native.new_http_archive(
      name = "iron_fit_behavior",
      build_file = str(Label("//third_party:iron_fit_behavior.BUILD")),
      sha256 = "10132a2ea309a37c4c07b8fead71f64abc588ee6107931e34680f5f36dd8291e",
      url = "https://github.com/PolymerElements/iron-fit-behavior/archive/v1.2.5.tar.gz",
      strip_prefix = "iron-fit-behavior-1.2.5",
  )

  native.new_http_archive(
      name = "iron_flex_layout",
      build_file = str(Label("//third_party:iron_flex_layout.BUILD")),
      sha256 = "79287f6ca1c2d4e003f68b88fe19d03a1b6a0011e2b4cae579fe4d1474163a2e",
      url = "https://github.com/PolymerElements/iron-flex-layout/archive/v1.3.0.tar.gz",
      strip_prefix = "iron-flex-layout-1.3.0",
  )

  native.new_http_archive(
      name = "iron_form_element_behavior",
      build_file = str(Label("//third_party:iron_form_element_behavior.BUILD")),
      sha256 = "1dd9371c638e5bc2ecba8a64074aa680dfb8712198e9612f9ed24d387efc8f26",
      url = "https://github.com/PolymerElements/iron-form-element-behavior/archive/v1.0.6.tar.gz",
      strip_prefix = "iron-form-element-behavior-1.0.6",
  )

  native.new_http_archive(
      name = "iron_icon",
      build_file = str(Label("//third_party:iron_icon.BUILD")),
      sha256 = "0ab579911bffaa52980110252c7d03ad2c888ae85b9784bd5e79ce23063eac3a",
      url = "https://github.com/PolymerElements/iron-icon/archive/v1.0.8.tar.gz",
      strip_prefix = "iron-icon-1.0.8",
  )

  native.new_http_archive(
      name = "iron_icons",
      build_file = str(Label("//third_party:iron_icons.BUILD")),
      sha256 = "3b18542c147c7923dc3a36b1a51984a73255d610f297d43c9aaccc52859bd0d0",
      url = "https://github.com/PolymerElements/iron-icons/archive/v1.1.3.tar.gz",
      strip_prefix = "iron-icons-1.1.3",
  )

  native.new_http_archive(
      name = "iron_iconset_svg",
      build_file = str(Label("//third_party:iron_iconset_svg.BUILD")),
      sha256 = "baa8721904308304024030ecd497546bf2cbf66021d7d121b0b65455af1aef5f",
      url = "https://github.com/PolymerElements/iron-iconset-svg/archive/v1.0.9.tar.gz",
      strip_prefix = "iron-iconset-svg-1.0.9",
  )

  native.new_http_archive(
      name = "iron_input",
      build_file = str(Label("//third_party:iron_input.BUILD")),
      sha256 = "c505101ead08ab25526b1f49baecc8c28b4221b92a65e7334c783bdc81553c36",
      url = "https://github.com/PolymerElements/iron-input/archive/1.0.10.tar.gz",
      strip_prefix = "iron-input-1.0.10",
  )

  native.new_http_archive(
      name = "iron_list",
      build_file = str(Label("//third_party:iron_list.BUILD")),
      sha256 = "c085f32a7dea17ef31b6ffb6e0453fbd564a237b0fede96d24d81abc7ae7a431",
      url = "https://github.com/PolymerElements/iron-list/archive/v1.1.7.tar.gz",
      strip_prefix = "iron-list-1.1.7",
  )

  native.new_http_archive(
      name = "iron_menu_behavior",
      build_file = str(Label("//third_party:iron_menu_behavior.BUILD")),
      sha256 = "5cb92efb9c15aa6711cee87a9ba5f51349b4a419dcc6eb5e74a99bfa4d33769b",
      url = "https://github.com/PolymerElements/iron-menu-behavior/archive/v1.1.8.tar.gz",
      strip_prefix = "iron-menu-behavior-1.1.8",
  )

  native.new_http_archive(
      name = "iron_meta",
      build_file = str(Label("//third_party:iron_meta.BUILD")),
      sha256 = "fb05e6031bae6b4effe5f15d44b3f548d5807f9e3b3aa2442ba17cf4b8b84361",
      url = "https://github.com/PolymerElements/iron-meta/archive/v1.1.1.tar.gz",
      strip_prefix = "iron-meta-1.1.1",
  )

  native.new_http_archive(
      name = "iron_overlay_behavior",
      build_file = str(Label("//third_party:iron_overlay_behavior.BUILD")),
      sha256 = "f23d14153d9e7f0312260ed6c44c09af848351fc31e514c0eb87681991e0739d",
      url = "https://github.com/PolymerElements/iron-overlay-behavior/archive/v1.7.6.tar.gz",
      strip_prefix = "iron-overlay-behavior-1.7.6",
  )

  native.new_http_archive(
      name = "iron_range_behavior",
      build_file = str(Label("//third_party:iron_range_behavior.BUILD")),
      sha256 = "b2f2b6d52284542330bd30b586e217926eb0adec5e13934a3cef557717c22dc2",
      url = "https://github.com/PolymerElements/iron-range-behavior/archive/v1.0.4.tar.gz",
      strip_prefix = "iron-range-behavior-1.0.4",
  )

  native.new_http_archive(
      name = "iron_resizable_behavior",
      build_file = str(Label("//third_party:iron_resizable_behavior.BUILD")),
      sha256 = "a87a78ee9223c2f6afae7fc94a3ff91cbce6f7e2a7ed3f2979af7945c9281616",
      url = "https://github.com/PolymerElements/iron-resizable-behavior/archive/v1.0.3.tar.gz",
      strip_prefix = "iron-resizable-behavior-1.0.3",
  )

  native.new_http_archive(
      name = "iron_selector",
      build_file = str(Label("//third_party:iron_selector.BUILD")),
      sha256 = "ba28a47443bad3b744611c9d7a79fb21dbdf2e35edc5ef8f812e2dcd72b16747",
      url = "https://github.com/PolymerElements/iron-selector/archive/v1.5.2.tar.gz",
      strip_prefix = "iron-selector-1.5.2",
  )

  native.new_http_archive(
      name = "iron_validatable_behavior",
      build_file = str(Label("//third_party:iron_validatable_behavior.BUILD")),
      sha256 = "aef4901e68043824f36104799269573dd345ffaac494186e466fdc79c06fdb63",
      url = "https://github.com/PolymerElements/iron-validatable-behavior/archive/v1.1.1.tar.gz",
      strip_prefix = "iron-validatable-behavior-1.1.1",
  )

  native.new_http_archive(
      name = "neon_animation",
      build_file = str(Label("//third_party:neon_animation.BUILD")),
      sha256 = "8800c314a76b2da190a2b203259c1091f6d38e0057ed37c2a3d0b734980fa9a5",
      url = "https://github.com/PolymerElements/neon-animation/archive/v1.2.2.tar.gz",
      strip_prefix = "neon-animation-1.2.2",
  )

  native.new_http_archive(
      name = "paper_behaviors",
      build_file = str(Label("//third_party:paper_behaviors.BUILD")),
      sha256 = "a4b64c9d4c210519c48acbe76a6fa4a3541bb92005852202e06f382b99e24ec5",
      url = "https://github.com/PolymerElements/paper-behaviors/archive/v1.0.11.tar.gz",
      strip_prefix = "paper-behaviors-1.0.11",
  )

  native.new_http_archive(
      name = "paper_button",
      build_file = str(Label("//third_party:paper_button.BUILD")),
      sha256 = "896c0a7e34bfcce63fc23c63e105ed9c4d62fa3a6385b7161e1e5cd4058820a6",
      url = "https://github.com/PolymerElements/paper-button/archive/v1.0.11.tar.gz",
      strip_prefix = "paper-button-1.0.11",
  )

  native.new_http_archive(
      name = "paper_checkbox",
      build_file = str(Label("//third_party:paper_checkbox.BUILD")),
      sha256 = "5a6f7e5446b49e38b914905ddef82e28908d15d88158108bfcb127252734b11a",
      url = "https://github.com/PolymerElements/paper-checkbox/archive/v1.1.3.tar.gz",
      strip_prefix = "paper-checkbox-1.1.3",
  )

  native.new_http_archive(
      name = "paper_dialog",
      build_file = str(Label("//third_party:paper_dialog.BUILD")),
      sha256 = "c6a9709e7f528d03dcd574503c18b72d4751ca30017346d16e6a791d37ed9259",
      url = "https://github.com/PolymerElements/paper-dialog/archive/v1.0.4.tar.gz",
      strip_prefix = "paper-dialog-1.0.4",
  )

  native.new_http_archive(
      name = "paper_dialog_behavior",
      build_file = str(Label("//third_party:paper_dialog_behavior.BUILD")),
      sha256 = "a7e0e27ce63554bc14f384cf94bcfa24da8dc5f5120dfd565f45e166261aee40",
      url = "https://github.com/PolymerElements/paper-dialog-behavior/archive/v1.2.5.tar.gz",
      strip_prefix = "paper-dialog-behavior-1.2.5",
  )

  native.new_http_archive(
      name = "paper_dropdown_menu",
      build_file = str(Label("//third_party:paper_dropdown_menu.BUILD")),
      sha256 = "7599b85cacd6e71d1f6c022343a9608458985ff3a5a915f8c3e863ab59c41b6a",
      url = "https://github.com/PolymerElements/paper-dropdown-menu/archive/v1.3.2.tar.gz",
      strip_prefix = "paper-dropdown-menu-1.3.2",
  )

  native.new_http_archive(
      name = "paper_header_panel",
      build_file = str(Label("//third_party:paper_header_panel.BUILD")),
      sha256 = "0db4bd8a4bf6f20dcd0dffb4f907b31c93a8647c9c021344239cf30b40b87075",
      url = "https://github.com/PolymerElements/paper-header-panel/archive/v1.1.4.tar.gz",
      strip_prefix = "paper-header-panel-1.1.4",
  )

  native.new_http_archive(
      name = "paper_icon_button",
      build_file = str(Label("//third_party:paper_icon_button.BUILD")),
      sha256 = "9408ca4dd98e78a08297c8dd1d287b92522023ad08cabb6e72d5321586c6bccb",
      url = "https://github.com/PolymerElements/paper-icon-button/archive/v1.1.1.tar.gz",
      strip_prefix = "paper-icon-button-1.1.1",
  )

  native.new_http_archive(
      name = "paper_input",
      build_file = str(Label("//third_party:paper_input.BUILD")),
      sha256 = "17c3dea9bb1c2026cc61324696c6c774214a0dc37686b91ca214a6af550994db",
      url = "https://github.com/PolymerElements/paper-input/archive/v1.1.18.tar.gz",
      strip_prefix = "paper-input-1.1.18",
  )

  native.new_http_archive(
      name = "paper_item",
      build_file = str(Label("//third_party:paper_item.BUILD")),
      sha256 = "12ee0dcb61b0d5721c5988571f6974d7b2211e97724f4195893fbcc9058cdac8",
      url = "https://github.com/PolymerElements/paper-item/archive/v1.1.4.tar.gz",
      strip_prefix = "paper-item-1.1.4",
  )

  native.new_http_archive(
      name = "paper_listbox",
      build_file = str(Label("//third_party:paper_listbox.BUILD")),
      sha256 = "3cb35f4fe9a3f15185a9e91711dba8f27e9291c8cd371ebf1be21b8f1d5f65fb",
      url = "https://github.com/PolymerElements/paper-listbox/archive/v1.1.2.tar.gz",
      strip_prefix = "paper-listbox-1.1.2",
  )

  native.new_http_archive(
      name = "paper_material",
      build_file = str(Label("//third_party:paper_material.BUILD")),
      sha256 = "09f6c8bd6ddbea2be541dc86306efe41cdfb31bec0b69d35a5dc29772bbc8506",
      url = "https://github.com/PolymerElements/paper-material/archive/v1.0.6.tar.gz",
      strip_prefix = "paper-material-1.0.6",
  )

  native.new_http_archive(
      name = "paper_menu",
      build_file = str(Label("//third_party:paper_menu.BUILD")),
      sha256 = "a3cee220926e315f7412236b3628288774694447c0da4428345f36d0f127ba3b",
      url = "https://github.com/PolymerElements/paper-menu/archive/v1.2.2.tar.gz",
      strip_prefix = "paper-menu-1.2.2",
  )

  native.new_http_archive(
      name = "paper_menu_button",
      build_file = str(Label("//third_party:paper_menu_button.BUILD")),
      sha256 = "b7797c81562590545679f0c6fa730f5e386b9f7ddc5f3f50998b5377f1264958",
      url = "https://github.com/PolymerElements/paper-menu-button/archive/v1.5.0.tar.gz",
      strip_prefix = "paper-menu-button-1.5.0",
  )

  native.new_http_archive(
      name = "paper_progress",
      build_file = str(Label("//third_party:paper_progress.BUILD")),
      sha256 = "2b6776b2f023c1f344feea17ba29b58d879e46f8ed43b7256495054b5183fff6",
      url = "https://github.com/PolymerElements/paper-progress/archive/v1.0.9.tar.gz",
      strip_prefix = "paper-progress-1.0.9",
  )

  native.new_http_archive(
      name = "paper_radio_button",
      build_file = str(Label("//third_party:paper_radio_button.BUILD")),
      sha256 = "6e911d0c308aa388136b3af79d1bdcbe5a1f4159cbc79d71efb4ff3b6c0b4e91",
      url = "https://github.com/PolymerElements/paper-radio-button/archive/v1.1.2.tar.gz",
      strip_prefix = "paper-radio-button-1.1.2",
  )

  native.new_http_archive(
      name = "paper_radio_group",
      build_file = str(Label("//third_party:paper_radio_group.BUILD")),
      sha256 = "7885ad1f81e9dcc03dcea4139b54a201ff55c18543770cd44f94530046c9e163",
      url = "https://github.com/PolymerElements/paper-radio-group/archive/v1.0.9.tar.gz",
      strip_prefix = "paper-radio-group-1.0.9",
  )

  native.new_http_archive(
      name = "paper_ripple",
      build_file = str(Label("//third_party:paper_ripple.BUILD")),
      sha256 = "ba76bfb1c737260a8a103d3ca97faa1f7c3288c7db9b2519f401b7a782147c09",
      url = "https://github.com/PolymerElements/paper-ripple/archive/v1.0.5.tar.gz",
      strip_prefix = "paper-ripple-1.0.5",
  )

  native.new_http_archive(
      name = "paper_slider",
      build_file = str(Label("//third_party:paper_slider.BUILD")),
      sha256 = "08e7c541dbf5d2e959208810bfc03188e82ced87e4d30d325172967f67962c3c",
      url = "https://github.com/PolymerElements/paper-slider/archive/v1.0.10.tar.gz",
      strip_prefix = "paper-slider-1.0.10",
  )

  native.new_http_archive(
      name = "paper_styles",
      build_file = str(Label("//third_party:paper_styles.BUILD")),
      sha256 = "6d26b0a4c286402098853dc7388f6b22f30dfb7a74e47b34992ac03380144bb2",
      url = "https://github.com/PolymerElements/paper-styles/archive/v1.1.4.tar.gz",
      strip_prefix = "paper-styles-1.1.4",
  )

  native.new_http_archive(
      name = "paper_tabs",
      build_file = str(Label("//third_party:paper_tabs.BUILD")),
      sha256 = "c4c5d9e4d9438123cbe3e3521e1013c6dd0b9cbe1d39c4614cdecc414614709b",
      url = "https://github.com/PolymerElements/paper-tabs/archive/v1.6.2.tar.gz",
      strip_prefix = "paper-tabs-1.6.2",
  )

  native.new_http_archive(
      name = "paper_toggle_button",
      build_file = str(Label("//third_party:paper_toggle_button.BUILD")),
      sha256 = "b6aa110fc76419c3c802050b5db11f038e5f0d14fb6d10cee626306e310cb8b3",
      url = "https://github.com/PolymerElements/paper-toggle-button/archive/v1.1.2.tar.gz",
      strip_prefix = "paper-toggle-button-1.1.2",
  )

  native.new_http_archive(
      name = "paper_toolbar",
      build_file = str(Label("//third_party:paper_toolbar.BUILD")),
      sha256 = "dbddffc0654d9fb5fb48843087eebe16bf7a134902495a664c96c11bf8a2c63d",
      url = "https://github.com/PolymerElements/paper-toolbar/archive/v1.1.4.tar.gz",
      strip_prefix = "paper-toolbar-1.1.4",
  )

  native.new_http_archive(
      name = "paper_tooltip",
      build_file = str(Label("//third_party:paper_tooltip.BUILD")),
      sha256 = "4c6667acf01f73da14c3cbc0aa574bf14280304567987ee0314534328377d2ad",
      url = "https://github.com/PolymerElements/paper-tooltip/archive/v1.1.2.tar.gz",
      strip_prefix = "paper-tooltip-1.1.2",
  )

  native.new_http_archive(
      name = "polymer",
      build_file = str(Label("//third_party:polymer.BUILD")),
      sha256 = "188fbc766f0fb3831dfd14b0c6cf80fe7bffb48fe52eee178bfb815abc834e9f",
      url = "https://github.com/polymer/polymer/archive/v1.6.1.tar.gz",
      strip_prefix = "polymer-1.6.1",
  )

  native.new_http_archive(
      name = "promise_polyfill",
      build_file = str(Label("//third_party:promise_polyfill.BUILD")),
      sha256 = "4495450e5d884c3e16b537b43afead7f84d17c7dc061bcfcbf440eac083e4ef5",
      url = "https://github.com/PolymerLabs/promise-polyfill/archive/v1.0.0.tar.gz",
      strip_prefix = "promise-polyfill-1.0.0",
  )

  ##############################################################################
  # TensorBoard Polymer Demo Dependencies

  native.new_http_archive(
      name = "iron_demo_helpers",
      build_file = str(Label("//third_party:iron_demo_helpers.BUILD")),
      sha256 = "aa7458492a6ac3d1f6344640a4c2ab07bce64e7ad0422b83b5d665707598cce6",
      url = "https://github.com/PolymerElements/iron-demo-helpers/archive/v1.1.0.tar.gz",
      strip_prefix = "iron-demo-helpers-1.1.0",
  )

  native.new_http_archive(
      name = "marked_element",
      build_file = str(Label("//third_party:marked_element.BUILD")),
      sha256 = "7547616df95f8b903757e6afbabfcdba5322c2bcec3f17c726b8bba5adf4bc5f",
      url = "https://github.com/PolymerElements/marked-element/archive/v1.1.3.tar.gz",
      strip_prefix = "marked-element-1.1.3",
  )

  native.new_http_archive(
      name = "prism_element",
      build_file = str(Label("//third_party:prism_element.BUILD")),
      sha256 = "ad70bf9cd5bbdf525d465e1b0658867ab4022193eb9c74087a839044b46312b4",
      url = "https://github.com/PolymerElements/prism-element/archive/1.0.4.tar.gz",
      strip_prefix = "prism-element-1.0.4",
  )
