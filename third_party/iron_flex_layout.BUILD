package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_flex_layout",
    srcs = [
        "classes/iron-flex-layout.html",
        "classes/iron-shadow-flex-layout.html",
        "iron-flex-layout.html",
        "iron-flex-layout-classes.html",
        "@d3_d_ts//file",
    ],
    path = "/iron-flex-layout",
    deps = ["@polymer//:polymer"],
)
