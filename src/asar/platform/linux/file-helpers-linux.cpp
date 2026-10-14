#include "platform/file-helpers.h"
#include <sys/stat.h>

bool file_exists(const char * filename)
{
	struct stat st;
	return (!stat(filename, &st));
}

bool path_is_absolute(const char* path)
{
	return ('/' == path[0]);
}

char get_native_path_separator()
{
	return '/';
}

bool get_file_stamp(const char* path, file_stamp* out)
{
	struct stat st;
	if (stat(path, &st)) return false;
	out->regular = S_ISREG(st.st_mode);
	out->size = (unsigned long long)st.st_size;
	out->inode = (unsigned long long)st.st_ino;
	out->dev = (unsigned long long)st.st_dev;
#if defined(__APPLE__)
	out->mtime_ns = (unsigned long long)st.st_mtimespec.tv_sec * 1000000000ull + (unsigned long long)st.st_mtimespec.tv_nsec;
#else
	out->mtime_ns = (unsigned long long)st.st_mtim.tv_sec * 1000000000ull + (unsigned long long)st.st_mtim.tv_nsec;
#endif
	return true;
}

bool check_is_regular_file(const char* path)
{
	struct stat finfo;
	if (stat(path, &finfo) == 0)
	{
		// either regular file or symlink
		if (finfo.st_mode & (S_IFREG | S_IFLNK))
			return true;
	}
	return false;
}
