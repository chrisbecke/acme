#pragma once
#include <assert.h>

namespace anode {

  // a linked list
  template<typename item_t>
  class list {
    struct node {
      node* next;
      item_t data;
    }*_root;
  public:
    list() :_root(nullptr){}
    ~list(){
      for (node* scan = _root; scan;){
        node* todelete = scan;
        scan = scan->next;
        delete todelete;
      }
    }
    operator bool(){
      return _root != nullptr; // at least one element
    }

    // item_t must implement operator <
    list& operator +=(const item_t& itemToAdd)
    {
      // insert at back, insert at front, insert sorted.
      node** scan = &_root;
      while (*scan && (*scan)->data < itemToAdd)
        scan = &(*scan)->next;

      node* oldNext = *scan;
      // make sure to copy construct rather than default construct new items.
      *scan = new node{ oldNext, itemToAdd };
      return *this;
    }

    struct iterator {
      node* _node;
      bool operator !=(const iterator& other){
        return _node != other._node;
      }
      iterator operator++(){
        _node = _node->next;
        return *this;
      }
      const item_t& operator*(){
        return _node->data;
      }
    };
    iterator begin(){
      return{ _root };
    }
    iterator end(){
      return{ nullptr };
    }

    struct const_iterator {
      node* _node;
      bool operator !=(const const_iterator& other){
        return _node != other._node;
      }
      const_iterator operator++(){
        _node = _node->next;
        return *this;
      }
      const item_t& operator*(){
        return _node->data;
      }
    };
    const_iterator begin() const{
      return{ _root };
    }
    const_iterator end() const{
      return{ nullptr };
    }

  };

  template<typename key_t, typename item_t>
  class map {
    struct pair {
      key_t key;
      item_t value;
    };
    struct node {
      node* next;
      pair data;
    }*_root;
  public:
    map() :_root(nullptr){}
    ~map(){
      for (node* scan = _root; scan;){
        node* todelete = scan;
        scan = scan->next;
        delete todelete;
      }
    }
    operator bool()const{
      return _root != nullptr; // at least one element
    }

    // retrieves a settable item_t - adds a new node if necessary
    item_t& operator[](const key_t& key){
      node* item = findOrMake(key);
      return item->data.value;
    }

    map& operator -=(key_t key){
      removeFirstFound(key);
      return *this;
    }

    // iterator stuff
    struct iterator {
      node* _node;
      bool operator !=(const iterator& other){
        return this != &other._node;
      }
      iterator operator++(){
        return{ _node->next };
      }
      const pair& operator*(){
        return _node->data;
      }
    };
    iterator begin(){
      return{ _root };
    }
    iterator end(){
      return{ nullptr };
    }
    // iterator stuff
    struct const_iterator {
      const node* _node;
      bool operator !=(const const_iterator& other){
        return _node != other._node;
      }
      const_iterator& operator++(){
        _node = _node->next;
        return *this;
      }
      const pair& operator*(){
        return _node->data;
      }
    };
    const_iterator begin() const{
      return{ _root };
    }
    const_iterator end() const{
      return{ nullptr };
    }
  protected:
    node* findOrMake(const key_t& key){
      node** scan;
      for ( scan = &_root; *scan; scan = &(*scan)->next)
      {
        if ((*scan)->data.key == key)
          return *scan;
      }
      // we have reached the end.
      *scan = new node;
      (*scan)->next = nullptr;
      (*scan)->data.key = key;
      return *scan;
    }
    void removeFirstFound(const key_t& key){
      node** scan;
      for (scan = &_root; *scan; scan = &(*scan)->next)
      {
        if ((*scan)->data.key == key){
          node* toRemove = *scan;
          *scan = (*scan)->next;
          delete toRemove;
          return;
        }
      }
    }
  };

}