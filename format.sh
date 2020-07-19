#!/bin/bash

clang-format-7 -assume-filename=.clangformat -i x86/*
clang-format-7 -assume-filename=.clangformat -i cpp/*
clang-format-7 -assume-filename=.clangformat -i src/*.cc
