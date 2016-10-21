package(default_visibility = ["//tensorflow:internal"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # Apache 2.0

# bazel run //tensorflow/tensorboard/components/vz_line_chart/demo
webfiles(
    name = "demo",
    srcs = ["index.html"],
    path = "/vz-line-chart/demo",
    deps = [
        "//tensorflow/tensorboard/components/vz-line-chart:vz_line_chart",
        "@iron_demo_helpers//:iron_demo_helpers",
        "@paper_styles//:paper_styles",
        "@webcomponentsjs//:webcomponentsjs",
    ],
)
