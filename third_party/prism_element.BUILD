package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "prism_element",
    srcs = [
        "prism-highlighter.html",
        "prism-import.html",
    ],
    path = "/prism-element",
    deps = [
        "@polymer//:polymer",
        "@prism//:prism",
    ],
)
