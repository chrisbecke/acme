#pragma once
#include <stdint.h>

namespace acme {

  struct internet_header
  {
    uint8_t ihl : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t total_len;
    //
    uint16_t id;
    uint16_t frag_off : 13,
             flags : 3;
    //
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    //
    uint32_t source_address;
    //
    uint32_t destination_address;
    //
    uint32_t options;
  };

  struct tcp_header
  {
    uint16_t source_port;
    uint16_t dest_port;

  };

  struct udp_header
  {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t udp_length;
    uint16_t udp_checksum;
  };


};