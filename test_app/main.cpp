#include "serial.h"
#include <iostream>

enum class States{INIT = 0,CALC_SHA256 = 1,EXIT = 2,MAX_VAL};
int main(int argc, char **argv)
{
  std::string port = "/dev/mytty";
  bool is_running = true;
  States state = States::INIT;
  
  std::string text;
  Serial my_serial(port,9600);
  std::cout<<"Started test app"<<std::endl;
  while(is_running){
    switch (state)
    {
      case States::INIT:{
        std::cout<< "Enter operation type"<<std::endl;
        std::cout<<
                    "1-Calc_sha256\n"
                    "2-Exit\n" << std::endl;
      int choice;
      std::cin >> choice;
      state = static_cast<States>(choice % static_cast<int>(States::MAX_VAL));
      }break;

      case States::CALC_SHA256:{
        std::cout<< "Enter string to calculate sha256"<<std::endl;
        std::cin>>text;      
        if(!text.empty()){
          std::cout<<"Write operation started"<<std::endl;

          my_serial.writeString(text);
          std::cout<<"Text "<<text.c_str()<<" sent"<<std::endl;
          std::string hash;
          hash = my_serial.readLine();
          if(!hash.empty()){
            std::cout<<"Calculated sha256: " << hash.c_str() << std::endl;
          }
        }
        else{
          std::cout<< "Text is empty"<< std::endl;
        }
        state = States::INIT;
      }break;
      case States::EXIT:{
        std::cout<<"Exiting app"<<std::endl;
        is_running = false;
      }break;
      default:{
        std::cout <<"Not valid choice"<<std::endl;
        state = States::INIT;
      }break;
    }
  }

  return 0;
}
