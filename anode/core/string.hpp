#pragma once
#include <stdlib.h>
//#include <memory.h>
//#include <stdio.h>
#include <string.h>

namespace anode {

  class string
  {
  protected:
    char* str;
  public:
    string(){
      str = nullptr;
    }
    string(const char* src, int len){
      str = (char*)malloc(len + 1);
      memcpy(str, src, len);
      str[len] = 0;
    }
    string(const string& rhs){
      str = _strdup(rhs.str);
    }
    string(string&& rhs){
      str = rhs.str;
      rhs.str = nullptr;
    }
    ~string(){
      free(str);
    }
    string& operator=(const string& rhs){
      free(str);
      str = _strdup(*rhs);
      return *this;
    }
    string& operator=(string&& rhs){
      free(str);
      str = rhs.str;
      rhs.str = nullptr;
      return *this;
    }
    const char* operator*() const{
      return str?str:"";
    }
    // methods
    int length() const
    {
      return str?strlen(str):0;
    }
  };

  class string_buffer : public string
  {
  protected:
    int _reserve;
  public:
    string_buffer(int len){
      _reserve = len;
      str = (char*)malloc(len + 1);
      str[0] = 0;
    }
    operator char*(){
      return str;
    }
    int size()const{
      return _reserve;
    }
  };
}
