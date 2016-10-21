package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "webcomponentsjs",
    testonly = 1,
    srcs = [
        "CustomElements.js",
        "CustomElements.min.js",
        "HTMLImports.js",
        "HTMLImports.min.js",
        "MutationObserver.js",
        "MutationObserver.min.js",
        "ShadowDOM.js",
        "ShadowDOM.min.js",
        "webcomponents.js",
        "webcomponents.min.js",
        "webcomponents-lite.js",
        "webcomponents-lite.min.js",
    ],
    path = "/webcomponentsjs",
)
