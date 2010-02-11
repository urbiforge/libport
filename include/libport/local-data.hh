/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef LIBPORT_LOCAL_DATA_HH
# define LIBPORT_LOCAL_DATA_HH

# include <boost/function.hpp>

namespace libport
{
  namespace localdata
  {
    /// \brief Traits used by LocalData to define how to access its data.
    template <typename T, typename Encapsulate>
    struct Wrapper;
  }

  /// \brief Abstract class to handle local data form a piece of code which
  /// is not aware of the encapsulation.
  template <typename T>
  class AbstractLocalData
  {
  public:
    virtual T* get() = 0;
    virtual void set(T* v) = 0;
    virtual ~AbstractLocalData();
  };

  /// \brief Map a value of type \p T for each encapsulation corresponding
  /// to \p Enc.
  template <typename T, typename Enc>
  class LocalData
    : public AbstractLocalData<T>
  {
  public:
    /// Access the local value independently of the encapsulation.
    T* get();
    /// Define the local value independently of the encapsulation.
    void set(T* v);

  private:
    typedef typename localdata::Wrapper<T, Enc> traits;
    typedef typename traits::container container;
    /// Container corresponding to the encapsulation of the data.
    container container_;
  };

  /// \brief Abstract class to handle local singleton from a piece of code
  /// which is not aware of the encapsulation.
  template <typename T>
  class AbstractLocalSingleton
  {
  public:
    /// Type of the function used to build the value.
    typedef boost::function0<T*> builder;
    virtual T& instance(builder b) = 0;
    virtual ~AbstractLocalSingleton();
  };

  /// \brief Map a value of type \p T for each encapsulation corresponding
  /// to \p Enc.  The instance is uniq per process and will not change
  /// during the whole process.
  template <typename T, typename Enc>
  class LocalSingleton
    : public AbstractLocalSingleton<T>
  {
  public:
    typedef typename AbstractLocalSingleton<T>::builder builder;
    /// Get the instance of the current process and build it with \a b if
    /// its does not exists yet.
    T& instance(builder b);

  private:
    LocalData<T, Enc> data_;
  };
}

# include <libport/local-data.hxx>

#endif
