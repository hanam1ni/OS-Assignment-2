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

Alternate Installation
----------------------

If you are not able to use Meson and Ninja, please report this to the
libfuse mailing list. Until the problem is resolved, you may fall back
to an in-source build using autotools:

    $ ./configure
    $ make
    $ sudo make install

Note that support for building with autotools may disappear at some
point, so if you depend on using autotools for some reason please let
the libfuse developers know!


Security implications
---------------------

The *fusermount3* program is installed setuid root. This is done to
allow normal users to mount their own filesystem implementations.

To limit the harm that malicious users can do this way, *fusermount3*
enforces the following limitations:

  - The user can only mount on a mountpoint for which he has write
    permission

  - The mountpoint must not be a sticky directory which isn't owned by
    the user (like /tmp usually is)

  - No other user (including root) can access the contents of the
    mounted filesystem (though this can be relaxed by allowing the use
    of the *allow_other* and *allow_root* mount options in
    */etc/fuse.conf*)


If you intend to use the *allow_other* mount options, be aware that
FUSE has an unresolved [security
bug](https://github.com/libfuse/libfuse/issues/15): if the
*default_permissions* mount option is not used, the results of the
first permission check performed by the file system for a directory
entry will be re-used for subsequent accesses as long as the inode of
the accessed entry is present in the kernel cache - even if the
permissions have since changed, and even if the subsequent access is
made by a different user. This is of little concern if the filesystem
is accessible only to the mounting user (which has full access to the
filesystem anyway), but becomes a security issue when other users are
allowed to access the filesystem (since they can exploit this to
perform operations on the filesystem that they do not actually have
permissions for).

This bug needs to be fixed in the Linux kernel and has been known
since 2006 but unfortunately no fix has been applied yet. If you
depend on correct permission handling for FUSE file systems, the only
workaround is to use `default_permissions` (which does not currently
support ACLs), or to completely disable caching of directory entry
attributes.

Building your own filesystem
------------------------------

FUSE comes with several example file systems in the `examples`
directory. For example, the *passthrough* examples mirror the contents
of the root directory under the mountpoint. Start from there and adapt
the code!

The documentation of the API functions and necessary callbacks is
mostly contained in the files `include/fuse.h` (for the high-level
API) and `include/fuse_lowlevel.h` (for the low-level API). An
autogenerated html version of the API is available in the `doc/html`
directory and at http://libfuse.github.io/doxygen.


We build file system from libfuse
------------

If you need help about libfuse library, please ask on the <fuse-devel@lists.sourceforge.net>
mailing list (subscribe at
https://lists.sourceforge.net/lists/listinfo/fuse-devel).
