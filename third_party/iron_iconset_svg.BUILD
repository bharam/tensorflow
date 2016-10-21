package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_iconset_svg",
    srcs = ["iron-iconset-svg.html"],
    path = "/iron-iconset-svg",
    deps = [
        "@iron_meta//:iron_meta",
        "@polymer//:polymer",
    ],
)
