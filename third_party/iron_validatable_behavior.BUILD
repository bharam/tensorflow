package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_validatable_behavior",
    srcs = ["iron-validatable-behavior.html"],
    path = "/iron-validatable-behavior",
    deps = [
        "@iron_meta//:iron_meta",
        "@polymer//:polymer",
    ],
)
