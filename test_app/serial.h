#ifndef _SERIAL_H_
#define _SERIAL_H_
#include "asio.hpp"

class Serial
{
 private:
  asio::io_service m_io;
  asio::serial_port m_port;

 public:
  Serial(std::string port, unsigned int baud_rate);
  void writeString(std::string s);
  std::string readLine();
};


#endif
