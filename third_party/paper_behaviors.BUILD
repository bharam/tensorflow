package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "paper_behaviors",
    srcs = [
        "paper-button-behavior.html",
        "paper-checked-element-behavior.html",
        "paper-inky-focus-behavior.html",
        "paper-ripple-behavior.html",
    ],
    path = "/paper-behaviors",
    deps = [
        "@iron_behaviors//:iron_behaviors",
        "@iron_checked_element_behavior//:iron_checked_element_behavior",
        "@paper_ripple//:paper_ripple",
        "@polymer//:polymer",
    ],
)
