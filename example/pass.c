/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>


static int xmp_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = lstat(path, stbuf);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
        int res;
        /* On Linux this could just be 'mknod(path, mode, rdev)' but this
           is more portable */
        if (S_ISREG(mode)) {
                res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
                if (res >= 0)
                        res = close(res);
        } else if (S_ISFIFO(mode))
                res = mkfifo(path, mode);
        else
                res = mknod(path, mode, rdev);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{

        return 0;
}

static int xmp_unlink(const char *path)
{
        int res;
        res = unlink(path);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_rmdir(const char *path)
{
        int res;
        res = rmdir(path);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_rename(const char *from, const char *to, unsigned int flags)
{
        int res;
        if (flags)
                return -EINVAL;
        res = rename(from, to);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_chmod(const char *path, mode_t mode,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = chmod(path, mode);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = lchown(path, uid, gid);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_truncate(const char *path, off_t size,
                        struct fuse_file_info *fi)
{
        int res;
        if (fi != NULL)
                res = ftruncate(fi->fh, size);
        else
                res = truncate(path, size);
        if (res == -1)
                return -errno;
        return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
        int fd;
        int res;
        if(fi == NULL)
                fd = open(path, O_RDONLY);
        else
                fd = fi->fh;

        if (fd == -1)
                return -errno;
        res = pread(fd, buf, size, offset);
        if (res == -1)
                res = -errno;
        if(fi == NULL)
                close(fd);
        return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
        int fd;
        int res;
        (void) fi;
        if(fi == NULL)
                fd = open(path, O_WRONLY);
        else
                fd = fi->fh;

        if (fd == -1)
                return -errno;
        res = pwrite(fd, buf, size, offset);
        if (res == -1)
                res = -errno;
        if(fi == NULL)
                close(fd);
        return res;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
        (void) path;
        close(fi->fh);
        return 0;
}
static int xmp_fsync(const char *path, int isdatasync,
                     struct fuse_file_info *fi)
{
        /* Just a stub.  This method is optional and can safely be left
           unimplemented */
        (void) path;
        (void) isdatasync;
        (void) fi;
        return 0;
}

static struct fuse_operations OP = {
        .getattr        = xmp_getattr,
        .mknod          = xmp_mknod,
        .mkdir          = xmp_mkdir,
        .unlink         = xmp_unlink,
        .rmdir          = xmp_rmdir,
        .rename         = xmp_rename,
        .chmod          = xmp_chmod,
        .chown          = xmp_chown,
        .truncate       = xmp_truncate,
        .open           = xmp_open,
        .read           = xmp_read,
        .write          = xmp_write,
        .statfs         = xmp_statfs,
        .release        = xmp_release,
        /*.opendir 	=
        .readdir	=
        .releasedir =
        .fsync 		=
        .fsyncdir 	= */


};
int main(int argc, char *argv[])
{
  char *path[2];
  /*  char* tmp_img;
    char* tmp_path;
    tmp_img = argv[1];
    tmp_path = argv[2];*/

    char* tmp_img;
    char* tmp_path;
	char *path[2];
	tmp_img = argv[1];
    tmp_path = argv[2];


    char* cmd[500];
    sprintf(cmd,"mount %s %s",argv[1],argv[2]);
    system(cmd);



	path[0] = argv[0];
	path[1] = argv[1];
    fuse_main(2,path,&OP,NULL);

}
