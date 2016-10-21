package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_icon",
    srcs = ["iron-icon.html"],
    path = "/iron-icon",
    deps = [
        "@iron_flex_layout//:iron_flex_layout",
        "@iron_meta//:iron_meta",
        "@polymer//:polymer",
    ],
)
