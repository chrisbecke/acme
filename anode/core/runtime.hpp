#pragma once
#include <assert.h>

#include "collections.hpp"
#include "function.hpp"


namespace anode {

  class Object;

  template<class function_t>
  class Event;

  template <class ...params_t >
  class Event<void(params_t...)>
  {
    list<function<void(params_t...)>> _callbacks;
  public:
    Event& operator+=(function<void(params_t...)> callback){
      _callbacks+=(callback);
      return *this;
    }
    void operator()(params_t ... params){
      for (auto callback : _callbacks){
        callback(params...);
      }
    }
    operator bool(){
      return _callbacks.operator bool();
    }
  };

  namespace runtime {

    class Application {
      static Application* _instance;
    public:
      static Application* getInstance(){ return _instance; }
      Application(){
        assert(!_instance);
        _instance = this;
      }
      ~Application(){
        _instance = nullptr;
      }

      // AutoreleasePool things
      class AutoreleasePool {
      public:
        Object** _pool;
        int _len; // entries in pool
        int _max; // max entried before pool grows

        AutoreleasePool();
        ~AutoreleasePool();
        void addObject(Object* object);
        void drain();

      } Pool;
    };

  }
}

#include <functional>

namespace anode0
{
  typedef std::function<void()> RunLoop;

  class App {
    static RunLoop runLoop;
  public:
    static void schedule(RunLoop rl){
      runLoop = rl;
    }

    static int main(){
      if (runLoop)
        runLoop();
      return 0;
    }

  };
}

#include <vector>
namespace anode2
{


}
