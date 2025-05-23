cmake_minimum_required(VERSION 3.17)
project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        SOURCE_DIR "/Users/kaushisk/Documents/k-NN/jni/googletest-src"
        BINARY_DIR "/Users/kaushisk/Documents/k-NN/jni/googletest-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        )
