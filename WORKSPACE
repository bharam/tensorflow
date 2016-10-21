workspace(name = "org_tensorflow")

# Uncomment and update the paths in these entries to build the Android demo.
#android_sdk_repository(
#    name = "androidsdk",
#    api_level = 23,
#    build_tools_version = "23.0.1",
#    # Replace with path to Android SDK on your system
#    path = "<PATH_TO_SDK>",
#)
#
#android_ndk_repository(
#    name="androidndk",
#    path="<PATH_TO_NDK>",
#    api_level=21)

# Please add all new TensorFlow dependencies in workspace.bzl.
load("//tensorflow:workspace.bzl", "tf_workspace")

tf_workspace()

# Specify the minimum required bazel version.
load("//tensorflow:tensorflow.bzl", "check_version")

check_version("0.3.0")

local_repository(
    name = "io_bazel_rules_closure",
    path = "/usr/local/google/home/jart/code/rules_closure",
)

load("@io_bazel_rules_closure//closure:defs.bzl", "closure_repositories")

closure_repositories()
