#pragma once
#include <assert.h>

namespace anode {

  template<typename function_t>
  class function;

  template<typename ret_t, typename ... params_t>
  class function < ret_t(params_t ...) >
  {
    struct capture {
      virtual ret_t operator()(params_t ...) const = 0;
      virtual capture* copy() = 0;
    }*_capture;

    template<typename lambda_t>
    void captureLambda(lambda_t lambda){
      struct local_invoke : capture {
        lambda_t _lambda;
        local_invoke(lambda_t lambda) :_lambda(lambda){}
        ret_t operator()(params_t ... params) override {
          return _lambda(params ...);
        }
        capture* copy() override {
          return new local_invoke(_lambda);
        }
      };
      _capture = new local_invoke(lambda);
    }
  public:
    // the default, and the explicit nullptr constructor
    function(decltype(nullptr)=nullptr){
      _capture = nullptr;
    }
    template<typename lambda_t>
    function(lambda_t lambda){
      struct local_invoke : capture {
        lambda_t _lambda;
        local_invoke(lambda_t lambda) :_lambda(lambda){}
        ret_t operator()(params_t ... params) const override {
          return _lambda(params ...);
        }
        capture* copy() override {
          return new local_invoke(_lambda);
        }
      };
      _capture = new local_invoke(lambda);
    }
    function(const function& rvalue){
      _capture = rvalue._capture?rvalue._capture->copy():nullptr;
    }
    function(function&& rvalue){
      _capture = rvalue._capture;
      rvalue._capture = nullptr;
    }
    function& operator=(const function& rvalue){
      if (_capture == rvalue._capture)
        return *this;
      delete _capture;
      _capture = rvalue._capture->copy();
      return *this;
    }
    function& operator=(function&& rvalue){
      _capture = rvalue._capture;
      rvalue._capture = nullptr;
      return *this;
    }
    ~function(){
      delete _capture;
    }
    ret_t operator()(params_t ... params) const{
      return (*_capture)(params ...);
    }
    operator bool() const{
      return _capture != nullptr;
    }
  };

}