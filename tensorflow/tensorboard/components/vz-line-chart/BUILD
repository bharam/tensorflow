package(default_visibility = ["//tensorflow:internal"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # Apache 2.0

webfiles(
    name = "vz_line_chart",
    srcs = [
        "vz-line-chart.html",
        ":ts",
    ],
    path = "/vz-line-chart",
    deps = [
        "//tensorflow/tensorboard/components/tf-imports:lodash",
        "//tensorflow/tensorboard/components/tf-imports:plottable",
        "@polymer//:polymer",
    ],
)

genrule(
    name = "ts",
    srcs = [
        "@lib_es6_d_ts//file",
        "@d3_d_ts//file",
        "@plottable_d_ts//file",
        "@lodash_d_ts//file",
        "vz-chart-helpers.ts",
        "dragZoomInteraction.ts",
        "vz-line-chart.ts",
    ],
    outs = [
        "vz-chart-helpers.js",
        "dragZoomInteraction.js",
        "vz-line-chart.js",
    ],
    cmd = "$(location @nodejs//:bin/node)" +
          " $(location @tsc_js//file)" +
          " --inlineSourceMap" +
          " --inlineSources" +
          " --outDir $(@D)" +
          " $(SRCS)",
    tools = [
        "@nodejs//:bin/node",
        "@tsc_js//file",
    ],
)
