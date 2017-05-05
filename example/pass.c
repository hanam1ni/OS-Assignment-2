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

#include <time.h>
#include <utime.h>

int rev_time;
char* mount_path;

char* get_filename(char* str) {
    int i, index=strlen(str)-1;
    for(i=0; i<=index; index--) {
        if(str[index] == '/') {
            return &str[index]; 
        }
    }
    return NULL;
}

static int myfs_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = lstat(path, stbuf);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_mknod(const char *path, mode_t mode, dev_t rdev)
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

static int myfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}


static int myfs_mkdir(const char *path, mode_t mode)
{
  int res;
  char* tmp[500];
	res = mkdir(path, mode);
	if (res == -1)
		return -errno;
  sprintf(tmp,"%s/archive",path);
  res = mkdir(tmp, mode);
  if (res == -1)
		return -errno;
  return 0;
}

static int myfs_unlink(const char *path)
{
        int res;
        res = unlink(path);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_rmdir(const char *path)
{
        int res;
        res = rmdir(path);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_rename(const char *from, const char *to, unsigned int flags)
{
        int res;
        if (flags)
                return -EINVAL;
        res = rename(from, to);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_chmod(const char *path, mode_t mode,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = chmod(path, mode);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_chown(const char *path, uid_t uid, gid_t gid,
                     struct fuse_file_info *fi)
{
        (void) fi;
        int res;
        res = lchown(path, uid, gid);
        if (res == -1)
                return -errno;
        return 0;
}

static int myfs_truncate(const char *path, off_t size,
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

static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
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

static int myfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
        int fd;
        int res;
        (void) fi;
        char fullBackupPath[500];
        char fullMountPoint[500];
        time_t nowTime = time(NULL);
        int lastModifiedTime;
        int lastVersion = 1;
        struct stat fileStat;
        time_t secondLastModifiedTime;

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

        sprintf(fullMountPoint, "%s%s", mount_path, path);
        stat(fullMountPoint, &fileStat);

        secondLastModifiedTime = fileStat.st_mtime;

        do
        {
            sprintf(fullBackupPath, "%s%s%s%c%d", mount_path, "/archive/", path, '.', lastVersion);
            openFileforBackup = fopen(fullBackupPath, "r");
            if(!openFileforBackup) break;
            lastVersion++;
            fclose(openFileforBackup);

        } while (openFileforBackup);

        // retrieve for newest Last version number

        if (nowTime - secondLastModifiedTime > rev_time) // create snapshot
        {
            printf("Create snapshot for last version %d.\n", lastVersion);
        } else { // replace snapshot with new information
            printf("Don't snap file, Just replace new information to last Version.\n");
            if(lastVersion > 1) lastVersion--;
            sprintf(fullBackupPath, "%s%s%s%c%d", mount_path, "/archive/", path, '.', lastVersion);
        }

        res = open(fullBackupPath, O_CREAT | O_EXCL | O_WRONLY);
        fd  = open(fullBackupPath, O_WRONLY);
        res = pwrite(fd, buf, size, offset);
        close(fd);

        printf("Last time is %d\n", secondLastModifiedTime);

        return res;
}

static int myfs_release(const char *path, struct fuse_file_info *fi)
{
        (void) path;
        close(fi->fh);
        return 0;
}
static int myfs_fsync(const char *path, int isdatasync,
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
        .getattr        = myfs_getattr,
        .mknod          = myfs_mknod,
        .mkdir          = myfs_mkdir,
        .unlink         = myfs_unlink,
        .rmdir          = myfs_rmdir,
        .rename         = myfs_rename,
        .chmod          = myfs_chmod,
        .chown          = myfs_chown,
        .truncate       = myfs_truncate,
        .open           = myfs_open,
        .read           = myfs_read,
        .write          = myfs_write,
        .release        = myfs_release,
        /*.opendir 	=
        .readdir	=
        .releasedir =
        .fsync 		=
        .fsyncdir 	= */


};

int main(int argc, char *argv[])
{
  if(argc == 5 && !strcmp(argv[3], "-t")){
    char *tmp[2];
    tmp[0] = argv[0];
  	tmp[1] = argv[2];

    char* cmd[500];
    sprintf(cmd,"mount %s %s",argv[1],argv[2]);
    system(cmd);

    mount_path = argv[2; // create variable for mount path

    rev_time = atoi(argv[4]);

    fuse_main(2,tmp,&OP,NULL);
  }else{
    printf("\t./vcowfs <Image File> <Moaunt Point> -t <Auto-snapshot Delay (seconds)>\n");
  }
}
