package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # MIT

exports_files(["LICENSE"])

webfiles(
    name = "marked",
    srcs = ["lib/marked.js"],
    path = "/marked",
)
