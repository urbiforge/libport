#ifndef LIBPORT_UNISTD_H
# define LIBPORT_UNISTD_H

# include "detect_win32.h"

# include "libport/config.h"

// This is traditional Unix file.
# ifdef LIBPORT_HAVE_UNISTD_H
#  include "unistd.h"
# endif

// This seems to be its WIN32 equivalent.
// http://msdn2.microsoft.com/en-us/library/ms811896(d=printer).aspx#ucmgch09_topic7.
# if defined WIN32 || defined LIBPORT_WIN32
#  include "io.h"
# endif

#if defined LIBPORT_URBI_ENV_AIBO
static char *getcwd(char *buf, size_t size)
{
  if (buf == 0 || size <= 3)
    return 0;
  strncpy (buf, "/MS", size);
  return buf;
}
#else
# if defined LIBPORT_HAVE__GETCWD
#  define getcwd _getcwd
# elif !defined LIBPORT_HAVE_GETCWD
#  error I need either getcwd() or _getcwd()
# endif
#endif

#endif // !LIBPORT_UNISTD_H
