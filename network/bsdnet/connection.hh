#ifndef NETWORK_BSDNET_CONNECTION_HH
# define NETWORK_BSDNET_CONNECTION_HH

/** \file Connection.h.cpp
 *  \brief the linux specialization of the UConnection class of the URBI kernel.
 *  @author Anthony Truchet from a previous work by Arnaud Sarthou  */

# include <sys/types.h>

# include "kernel/utypes.hh"
# include "kernel/uconnection.hh"

# include "network/bsdnet/network.hh"

//! LinuxConnection implements an TCP/IP client connection.
class Connection : public UConnection, public Network::Pipe
{
public:
  // Parameters used by the constructor.
  enum
  {
    MINSENDBUFFERSIZE = 4096,
    MAXSENDBUFFERSIZE = 33554432,
    // This is also the size of the buffer
    PACKETSIZE	     = 16384,
    MINRECVBUFFERSIZE = 4096,
    MAXRECVBUFFERSIZE = 33554432,
  };

  //! Creates a new connection from the connection file descriptor
  /**
   * @param fd the file descriptor of the underlying socket
   * @param clientinfo a pointer to the informations about the client connected
   * NOTE : the LinuxConnection stole the property of the struct hostent and
   * is thus responsible for its deletion, and the declaration of its allocation*/
  Connection(int connfd);
  virtual ~Connection();
  virtual UConnection& close ();

  virtual std::ostream& print (std::ostream& o) const;

  /*ENABLE_URBI_MEM*/

  //! Called when the underlying fd is ready to be read
  void doRead();
  //! Called when the underlying fd is ready to be written
  void doWrite();

  virtual void notifyRead()
  {
    doRead();
  }
  virtual void notifyWrite()
  {
    doWrite();
  }
  virtual UConnection& send (const char* buffer, int length);

public:
  //! Accessor for the underlying file descriptor
  inline operator int() const
  {
    return fd;
  }

  virtual int readFD()
  {
    return fd;
  }

  virtual int writeFD()
  {
    return send_queue_empty() ? -1 : fd;
  }

protected:
  //! Overloading this function is requiered by UConnection
  virtual int effective_send (const char* buffer, int length);
  //! The file descriptor of the connection
  int fd;
  //! The reception buffer
  char read_buff[PACKETSIZE];

public:
  virtual UConnection& endline ();
};

#endif // !NETWORK_BSDNET_CONNECTION_HH
