#include "serial.h"


Serial::Serial(std::string port, unsigned int baud_rate)
  : m_io(), m_port(m_io,port)
{
  m_port.set_option(asio::serial_port_base::baud_rate(baud_rate));
}


void Serial::writeString(std::string s)
{
  
 asio::write(m_port,asio::buffer(s.c_str(),s.size()));

}

std::string Serial::readLine()
{
  char c;
  std::string result;
  
  for(;;){
    asio::read(m_port,asio::buffer(&c,1));
    switch(c){
      case '\r':
	      break;
      case '\n':{
	      return result;
      }
      default:{
	      result += c;
      }
	
    }
  }
}
