package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_demo_helpers",
    srcs = [
        "demo-pages-shared-styles.html",
        "demo-snippet.html",
    ],
    path = "/iron-demo-helpers",
    deps = [
        "@iron_flex_layout//:iron_flex_layout",
        "@iron_icons//:iron_icons",
        "@marked_element//:marked_element",
        "@paper_icon_button//:paper_icon_button",
        "@paper_styles//:paper_styles",
        "@polymer//:polymer",
        "@prism_element//:prism_element",
    ],
)
