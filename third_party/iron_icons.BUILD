package(default_visibility = ["//visibility:public"])

load("@io_bazel_rules_closure//closure:defs.bzl", "webfiles")

licenses(["notice"])  # BSD-3-Clause

exports_files(["LICENSE"])

webfiles(
    name = "iron_icons",
    srcs = [
        "av-icons.html",
        "communication-icons.html",
        "device-icons.html",
        "editor-icons.html",
        "hardware-icons.html",
        "image-icons.html",
        "iron-icons.html",
        "maps-icons.html",
        "notification-icons.html",
        "places-icons.html",
        "social-icons.html",
    ],
    path = "/iron-icons",
    deps = [
        "@iron_icon//:iron_icon",
        "@iron_iconset_svg//:iron_iconset_svg",
    ],
)
