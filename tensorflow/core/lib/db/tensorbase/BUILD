load("//tensorflow:tensorflow.bzl", "tf_cc_binary")

tf_cc_binary(
    name = "experiment",
    srcs = ["experiment.cc"],
    linkstatic = 1,
    deps = [
        "//tensorflow/core:direct_session",
        "//tensorflow/core:framework",
        "//tensorflow/core:framework_internal",
        "//tensorflow/core:lib",
        "//tensorflow/core:lib_internal",
        "//tensorflow/core/lib/db:sqlite",
    ],
)

tf_cc_binary(
    name = "preallocate",
    srcs = ["preallocate.cc"],
    linkstatic = 1,
    deps = [
        "//tensorflow/core:direct_session",
        "//tensorflow/core:framework",
        "//tensorflow/core:framework_internal",
        "//tensorflow/core:lib",
        "//tensorflow/core:lib_internal",
        "//tensorflow/core/lib/db:sqlite",
    ],
)

tf_cc_binary(
    name = "reservoir",
    srcs = ["reservoir.cc"],
    linkstatic = 1,
    deps = [
        "//tensorflow/core:direct_session",
        "//tensorflow/core:framework",
        "//tensorflow/core:framework_internal",
        "//tensorflow/core:lib",
        "//tensorflow/core:lib_internal",
        "//tensorflow/core/lib/db:sqlite",
    ],
)

tf_cc_binary(
    name = "noreservoir",
    srcs = ["noreservoir.cc"],
    linkstatic = 1,
    deps = [
        "//tensorflow/core:direct_session",
        "//tensorflow/core:framework",
        "//tensorflow/core:framework_internal",
        "//tensorflow/core:lib",
        "//tensorflow/core/lib/db:sqlite",
    ],
)
