package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "marked_element",
    srcs = [
        "marked-element.html",
        "marked-import.html",
    ],
    path = "/marked-element",
    deps = [
        "@marked//:marked",
        "@polymer//:polymer",
    ],
)
