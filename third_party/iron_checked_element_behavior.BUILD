package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_checked_element_behavior",
    srcs = ["iron-checked-element-behavior.html"],
    path = "/iron-checked-element-behavior",
    deps = [
        "@iron_form_element_behavior//:iron_form_element_behavior",
        "@iron_validatable_behavior//:iron_validatable_behavior",
        "@polymer//:polymer",
    ],
)
