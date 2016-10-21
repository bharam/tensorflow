package(default_visibility = ["//tensorflow:internal"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # Apache 2.0

webfiles(
    name = "d3",
    srcs = [
        "d3.html",
        "@d3_js//file",
    ],
    path = "/tf-imports",
)

webfiles(
    name = "lodash",
    srcs = [
        "lodash.html",
        "@lodash_js//file",
    ],
    path = "/tf-imports",
)

webfiles(
    name = "graphlib",
    srcs = [
        "graphlib.html",
        "@graphlib_core_js//file",
    ],
    path = "/tf-imports",
    deps = [":lodash"],
)

webfiles(
    name = "dagre",
    srcs = [
        "dagre.html",
        "@dagre_core_js//file",
    ],
    path = "/tf-imports",
    deps = [
        ":graphlib",
        ":lodash",
    ],
)

webfiles(
    name = "plottable",
    srcs = [
        "plottable.html",
        "@plottable_css//file",
        "@plottable_js//file",
    ],
    path = "/tf-imports",
    deps = [":d3"],
)
