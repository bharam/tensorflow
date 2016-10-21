package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "polymer",
    srcs = [
        "polymer.html",
        "polymer-micro.html",
        "polymer-mini.html",
    ],
    path = "/polymer",
)
