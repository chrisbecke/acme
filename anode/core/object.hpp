#pragma once
#include "runtime.hpp"
#include <assert.h>

namespace anode {

  template<class object_t>
  class Reference {
    object_t* _object;
  public:
    Reference(object_t* object=nullptr):_object(object){
      if (_object)
        _object->retain();
    }
    Reference(const Reference& other){
      if ((_object = other._object))
        _object->retain();
    }
    Reference(Reference&& other){
      _object = other._object;
      other._object = nullptr;
    }
    ~Reference(){
      if (_object)
        _object->release();
    }
    Reference& operator =(object_t* other){
      setObject(other);
      return *this;
    }
    Reference& operator =(const Reference& other){
      setObject(other._object);
      return *this;
    }
    operator bool()const{
      return _object != nullptr;
    }
    operator object_t*() const {
      return _object;
    }

  private:
    void setObject(object_t* object){
      if (_object == object)
        return;
      if (_object)
        _object->release();
      if ((_object = object))
        _object->retain();
    }
  };

  class Object {
    volatile unsigned long _ref;
  public:
    Object() :_ref(1){ autorelease(); }
    ~Object(){ assert(!_ref); }
  protected:
    template<class object_t> friend class Reference;
    friend class runtime::Application::AutoreleasePool;
    void retain(){
      _ref++;
    }
    void release(){
      if (!--_ref)
        delete this;
    }
    void autorelease();
  };

  typedef Reference<Object> ObjectRef;

  class Error {
  public:
  };

}

namespace anode0 {

  class Object {
    volatile long _ref;
  public:
    Object() :_ref(1){ }
    virtual ~Object(){ assert(!_ref); }
    long retain(){
      assert(_ref >= 0);
      return ++_ref;
    }
    long release(){
      assert(_ref > 0);
      if (--_ref)
        return _ref;
      delete this;
      return 0l;
    }
  };
}