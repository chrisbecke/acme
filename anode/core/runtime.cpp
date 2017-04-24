#include "runtime.hpp"
#include "object.hpp"

using namespace anode;
using namespace anode::runtime;

const int max_pool = 4096;



Application* Application::_instance = nullptr;

// AutoreleasePool things
//  Object* _pool;
//  int _len; // entries in pool
//  int _max; // max entried before pool grows

Application::AutoreleasePool::AutoreleasePool(){
  _pool = new Object*[max_pool];
  _len = 0;
  _max = max_pool;
}

Application::AutoreleasePool::~AutoreleasePool()
{

}

void Application::AutoreleasePool::addObject(Object* object)
{
  assert(_len < _max);
  _pool[_len++] = object;
}

void Application::AutoreleasePool::drain()
{
  for (int i = 0; i < _len; i++)
  {
    _pool[i]->release();
  }
  _len = 0;
}

void Object::autorelease(){
  auto app = Application::getInstance();
  if (app){
    app->Pool.addObject(this);
  }
}
