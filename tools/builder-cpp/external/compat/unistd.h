/* POSIX functions the conversion programs use, for the Microsoft compiler. */
#ifndef BUILDER_COMPAT_UNISTD_H
#define BUILDER_COMPAT_UNISTD_H
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
static __inline int truncate(const char *path, long long length)
{
	int fd, r;
	if (_sopen_s(&fd, path, _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0)
		return -1;
	r = _chsize_s(fd, length) == 0 ? 0 : -1;
	_close(fd);
	return r;
}
#endif
