/**
 ** \file libport/file-library.cc
 ** \brief Implements libport::file_library.
 */

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <cerrno>

#include <libport/detect-win32.h>
#include <libport/config.h>

#ifdef LIBPORT_HAVE_DIRECT_H
# include <direct.h>
#endif

#include <libport/contract.hh>
#include <libport/file-library.hh>
#include <libport/file-system.hh>
#include <libport/foreach.hh>
#include <libport/tokenizer.hh>
#include <libport/unistd.h>

namespace libport
{

  void
  file_library::push_cwd()
  {
    push_current_directory(get_current_directory());
  }


  file_library::file_library()
  {
    push_cwd();
  }

  file_library::file_library(const path& p)
  {
    push_cwd();
    // Then only process given path.
    push_back(p);
  }


  file_library::file_library(const std::string& lib, const char* sep)
  {
    push_cwd();
    push_back(lib, sep);
  }

  path
  file_library::ensure_absolute_path(const path& p) const
  {
    if (p.absolute_get())
      return p;
    else
      return current_directory_get() / p;
  }


  file_library::strings_type
  file_library::split(const std::string& lib, const char* sep)
  {
    WIN32_IF(bool split_on_colon = strchr(sep, ':'), /* Nothing */);
    strings_type res;
    foreach (const std::string& s, make_tokenizer(lib, sep, "",
                                                  boost::keep_empty_tokens))
    {
#ifdef WIN32
      // In case we split "c:\foo" into "c" and "\foo", glue them
      // together again.
      if (split_on_colon
          && s[0] == '\\'
          && !res.empty()
          && res.back().length() == 1)
        res.back() += ':' + s;
      else
#endif
        res.push_back(s);
    }
    return res;
  }

  void
  file_library::push_back(const path& p)
  {
    search_path_.push_back(ensure_absolute_path(p));
  }

  void
  file_library::push_back(const std::string& lib, const char* sep)
  {
    foreach (const std::string& s, split(lib, sep))
      if (!s.empty())
        push_back(s);
  }

  void
  file_library::push_front(const path& p)
  {
    search_path_.push_front(ensure_absolute_path(p));
  }

  void
  file_library::push_front(const std::string& lib, const char* sep)
  {
    foreach (const std::string& s, split(lib, sep))
      if (!s.empty())
        push_front(s);
  }

  void
  file_library::push_current_directory(const path& p)
  {
    // Ensure that path is absolute.
    current_directory_.push_front(p.absolute_get()
				   ? p
				   : current_directory_get() / p);
  }

  void
  file_library::pop_current_directory()
  {
    precondition(!current_directory_.empty());
    current_directory_.pop_front();
  }

  path
  file_library::current_directory_get() const
  {
    precondition(!current_directory_.empty());
    return *current_directory_.begin();
  }


  path
  file_library::find_file(const path& file) const
  {
    path directory = file.dirname();

    if (directory.absolute_get())
    {
      // If file is absolute, just check that it exists.
      if (!file.exists())
      {
        errno = ENOENT;
        throw Not_found();
      }
      else
        return directory;
    }
    else
    {
      // Does the file can be found in current directory?
      if (find_in_directory(current_directory_get(), file))
        return (current_directory_get() / file).dirname();
      else
        return find_in_search_path(directory, file.basename());
    }
  }

  bool
  file_library::find_in_directory(const path& dir,
                                  const std::string& file) const
  {
    return (dir / file).exists();
  }

  path
  file_library::find_in_search_path(const path& relative_path,
                                    const std::string& filename) const
  {
    // Otherwise start scanning the search path.
    foreach (const path& p, search_path_)
    {
      path checked_dir = p.absolute_get() ? p : current_directory_get() / p;
      checked_dir /= relative_path;
      if (find_in_directory(checked_dir, filename))
        return checked_dir;
    }

    // File not found in search path.
    errno = ENOENT;
    throw Not_found();
  }

  std::ostream&
  file_library::dump(std::ostream& ostr) const
  {
    ostr << ".";
    foreach (const path& p,  search_path_)
      ostr << ":" << p;
    return ostr;
  }
}
