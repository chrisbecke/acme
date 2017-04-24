#pragma once
#include "core/managed.hpp"
namespace anode {



  class Buffer {

    struct BufferData
    {
      unsigned char Bytes;
    };
    RefStruct<BufferData> _data;

  };
}