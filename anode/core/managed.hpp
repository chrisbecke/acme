#pragma once

namespace anode
{
  template<typename struct_t>
  class RefStruct
  {
    struct Data {
      struct_t _data;
      unsigned long _ref;
      Data(struct_t&& data) :_data(data){}

    }*_ptr;

  public:
    // default ctor
    RefStruct() :_ptr(nullptr){}
    // assign from construction
    RefStruct(struct_t&& other){
      _ptr = new Data(other);
    }
    RefStruct(const RefStruct& other){
      if (_ptr = other._ptr)
        _ptr->_ref++;
    }
    RefStruct(RefStruct&& other){
      _ptr = other._ptr;
      other._ptr = nullptr;
    }
    RefStruct& operator=(const RefStruct& other){
      setData(other._ptr);
      return *this;
    }
    RefStruct& operator=(RefStruct&& other){
      _ptr = other._ptr;
      other._ptr = nullptr;
      return *this;
    }
    RefStruct& operator=(decltype(nullptr)){
      setData(nullptr);
      return *this;
    }
    ~RefStruct(){
      setData(nullptr);
    }
    operator bool()const{
      return _ptr != nullptr;
    }
    // not valid if the object is null.
    Data& operator*(){
      return *_ptr->_data;
    }
    // safer.
    struct_t* operator->(){
      return _ptr->_data;
    }
  private:
    void setData(Data* ptr){
      // trivial self assignment
      if (_ptr == ptr)
        return;
      // unlink the existing buf
      if (_ptr && --_ptr->_ref)
        delete _ptr;
      // link the new buf
      if (_ptr = ptr)
        _ptr->_ref++;
    }
  };
}