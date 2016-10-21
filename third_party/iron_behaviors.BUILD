package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_behaviors",
    srcs = [
        "iron-button-state.html",
        "iron-control-state.html",
    ],
    path = "/iron-behaviors",
    deps = [
        "@iron_a11y_keys_behavior//:iron_a11y_keys_behavior",
        "@polymer//:polymer",
    ],
)
