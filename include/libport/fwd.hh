/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef LIBPORT_FWD_HH
# define LIBPORT_FWD_HH

#include <boost/version.hpp>

namespace libport
{
  // asio.hh;
  class ConcreteSocket;
  class Socket;

  // lockable.hh.
  class Lockable;

  // input-arguments.
  namespace opts
  {
    struct DataVisitor;
    struct Data;
    struct DataList;
  }

  // map.hh.
  template <class T, class N>
  class map;

  // package-info.hh.
  class PackageInfo;

  // path.hh.
  class path;

  // ref-pt.hh.
  template <class T> class RefPt;

  // select-const.hh.
  template <typename T> struct constify_traits;
  template <typename T> struct id_traits;

  // semaphore.hh.
  class Semaphore;

  // shared-ref.hh.
  template <typename T> class shared_ref;

  // symbol.hh.
  class Symbol;

  // xltdl.hh
  class xlt_advise;
  class xlt_handle;
}

namespace boost
{
  namespace asio
  {
#if BOOST_VERSION >= 106800
    class io_context;
    typedef io_context io_service;
#else
    class io_service;
#endif
  }
}

#endif // !LIBPORT_FWD_HH
