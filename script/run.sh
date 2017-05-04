#!/bin/bash
rm -rf ../build
mkdir ../build
meson ../ ../build
cd ../build
ninja
