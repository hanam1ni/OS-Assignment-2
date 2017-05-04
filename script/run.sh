#!/bin/bash
rm -rf ../build
mkdir ../build
meson ../ ../build
ninja ../build
