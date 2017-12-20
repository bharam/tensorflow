# Description:
#   Libraries for storing tensors in SQL databases.

package(default_visibility = ["//tensorflow:internal"])

licenses(["notice"])  # Apache 2.0

load("//tensorflow:tensorflow.bzl", "tf_cc_test")

cc_library(
    name = "sqlite",
    srcs = ["sqlite.cc"],
    hdrs = ["sqlite.h"],
    deps = [
        "//tensorflow/compiler/xla:statusor",
        "//tensorflow/core:lib",
        "@org_sqlite",
    ],
)

tf_cc_test(
    name = "sqlite_test",
    srcs = ["sqlite_test.cc"],
    deps = [
        ":sqlite",
        "//tensorflow/core:lib",
        "//tensorflow/core:test",
        "//tensorflow/core:test_main",
    ],
)

load("//tensorflow/core/lib/db:sqlite.bzl", "tf_sqlite_extension")

tf_sqlite_extension(
    name = "snapfn",
    srcs = ["snapfn.cc"],
    deps = [
        "@org_sqlite",
        "@snappy",
    ],
)

py_test(
    name = "snapfn_test",
    size = "small",
    srcs = ["snapfn_test.py"],
    data = ["libsnapfn.so"],
    srcs_version = "PY2AND3",
    deps = [
        "//tensorflow/python:client_testlib",
        "@org_sqlite//:python",
    ],
)

filegroup(
    name = "all_files",
    srcs = glob(["*"]),
    visibility = ["//tensorflow:__pkg__"],
)
