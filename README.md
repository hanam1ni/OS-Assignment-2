Assignment 2 - Versioning Copy-On-Write File System (vCowFS)
=======

Installation
------------

You can download libfuse from
https://github.com/libfuse/libfuse/releases. To build and install, we
recommend to use [Meson](http://mesonbuild.com/) (version 0.38 or
newer) and [Ninja](https://ninja-build.org).  After extracting the
libfuse tarball, create a (temporary) build directory and run Meson:

    $ md build; cd build
    $ meson ..

    $ ninja       // Create executable file
    $ cd example  // Change directory to executable file location

    $ dd if=/dev/zero of=storage.img bs=512k count=2  // Create storage image
    $ md tmp
    $ sudo ./vcowfs storage.img tmp -t 60             // Mount Filesystem reversion time every 60 seconds


We build file system from libfuse
------------

If you need help about libfuse library, please ask on the <fuse-devel@lists.sourceforge.net>
mailing list (subscribe at
https://lists.sourceforge.net/lists/listinfo/fuse-devel).
