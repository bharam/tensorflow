package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "paper_styles",
    srcs = [
        "classes/global.html",
        "classes/shadow.html",
        "classes/shadow-layout.html",
        "classes/typography.html",
        "color.html",
        "default-theme.html",
        "demo.css",
        "demo-pages.html",
        "paper-styles.html",
        "paper-styles-classes.html",
        "shadow.html",
        "typography.html",
    ],
    path = "/paper-styles",
    deps = [
        "@font_roboto//:font_roboto",
        "@iron_flex_layout//:iron_flex_layout",
        "@polymer//:polymer",
    ],
)
