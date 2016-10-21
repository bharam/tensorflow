package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "paper_icon_button",
    srcs = [
        "paper-icon-button.html",
        "paper-icon-button-light.html",
    ],
    path = "/paper-icon-button",
    deps = [
        "@iron_icon//:iron_icon",
        "@paper_behaviors//:paper_behaviors",
        "@paper_styles//:paper_styles",
        "@polymer//:polymer",
    ],
)
