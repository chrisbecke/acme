#pragma once
#include "netflow.h"
#include "tcpip.h"

namespace acme {

  using namespace netflow_v5;

  uint32_t SysUptime()
  {
    return GetTickCount();
  }

  uint32_t EpochTime()
  {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    uint64_t epocNano = uint64_t(ft.dwHighDateTime) << 32 | uint64_t(ft.dwLowDateTime);

    uint64_t winEpocS = epocNano / 10000000ll;

    // The windows epoc starts 1601, the unix epoc at 1970, so we subtract
    // that time interval in seconds.
    uint64_t unixEpocS = winEpocS - 11644473600ll;

    return uint32_t(unixEpocS);
  }

  class NetflowPacketBuilder
  {
    struct {
      netflow_header header;
      netflow_record records[max_flows];
    } _pkt;

  private:
    int FindMatchingFlow(const internet_header& header, int len)
    {
      if (SysUptime() > ntohl(_pkt.header.sys_uptime) + 1000)// ntohs(_pkt.header.sampling_interval))
        return max_flows;

      int count = ntohs(_pkt.header.count);

      for (int i = 0; i < count; i++)
      {
        if (_pkt.records[i].dstaddr != header.destination_address ||
          _pkt.records[i].srcaddr != header.source_address ||
          _pkt.records[i].prot != header.protocol)
          continue; // doesn't match source or destination protocols
        if (header.protocol == IPPROTO_TCP){
          const tcp_header* tcp = (const tcp_header*)((uint32_t*)(&header)+header.ihl);
          if (tcp->dest_port != _pkt.records[i].dstport ||
            tcp->source_port != _pkt.records[i].srcport)
            continue;
        }
        else if (header.protocol == IPPROTO_UDP){
          const udp_header* udp = (const udp_header*)((uint32_t*)(&header)+header.ihl);
          if (udp->dest_port != _pkt.records[i].dstport ||
            udp->source_port != _pkt.records[i].srcport)
            continue;
        }
        return i;
      }
      return count; // count is the index of the next flow.
    }

    uint16_t SnmpIndexOf(uint32_t address)
    {
      // 192.x.x.x
      bool isLan = false;
      if ((address & 0xff) == 0xc0)
        isLan = true;

      // 10.x.x.x
      if ((address & 0xff) == 0x0a)
        isLan = true;

      // if1 is usually on the lan size, if0 is wan
      return isLan ? 1 : 0;
    }


    void StartNewFlow(int flow, const internet_header& header, int len)
    {
      auto& rec = _pkt.records[flow];

      rec.srcaddr = header.source_address;
      rec.dstaddr = header.destination_address;
      rec.nexthop = 0;
      rec.input = htons(SnmpIndexOf(header.source_address));
      rec.output = htons(SnmpIndexOf(header.destination_address));
      rec.dPkts = htonl(1);
      rec.dOctets = htonl(len);
      rec.first = htonl(SysUptime());
      rec.last = htonl(SysUptime());
      //rec.srcport = ?
      //rec.dstport
      rec.pad1 = 0;
      rec.tcp_flags = header.flags;
      rec.prot = header.protocol;
      rec.tos = header.tos;
      rec.src_as = 0;
      rec.dst_as = 0;
      rec.src_mask = 24;
      rec.dst_mask = 24;
      rec.pad2 = 0;

      if (header.protocol == IPPROTO_TCP){
        const tcp_header* tcp = (const tcp_header*)((uint32_t*)(&header) + header.ihl);
        rec.srcport = tcp->source_port;
        rec.dstport = tcp->dest_port;
      }
      else if (header.protocol == IPPROTO_UDP) {
        const udp_header* udp = (const udp_header*)((uint32_t*)(&header) + header.ihl);
        rec.srcport = udp->source_port;
        rec.dstport = udp->dest_port;
      }
    }

    void AccumulateFlow(int flow, const internet_header& header, int len)
    {
      auto& rec = _pkt.records[flow];
//      rec.srcaddr = header.source_address;
//      rec.dstaddr = header.destination_address;
//      rec.nexthop = 0;
//      rec.input = SnmpIndexOf(header.source_address);
//      rec.output = SnmpIndexOf(header.destination_address);
      rec.dPkts = htonl( ntohl(rec.dPkts) +1);
      rec.dOctets = htonl( ntohl(rec.dOctets) + len);
//      rec.first = SysUptime();
      rec.last = htonl(SysUptime());
      //rec.srcport = ?
      //rec.dstport
      rec.tcp_flags |= header.flags;
      assert(rec.prot == header.protocol);
      //assert(rec.tos == header.tos); ?? changes over the life of a packet!?
      //rec.src_as = 0;
//      rec.dst_as = 0;
//      rec.src_mask = 24;
//      rec.dst_mask = 24;
    }

  public:
    NetflowPacketBuilder()
    {
      _pkt.header.version = htons(5);
      _pkt.header.count = 0;
      _pkt.header.sys_uptime = htonl(SysUptime());
      _pkt.header.unix_secs = htonl(EpochTime());
      _pkt.header.unix_nsecs = 0;
      _pkt.header.flow_sequence = 0;
      _pkt.header.engine_type = 0;
      _pkt.header.engine_id = 0;

      int interval = 10000;
      assert((interval & ~0x3fff) == 0);

      _pkt.header.sampling_interval = htons((1 << 14) | interval);
    }

    template<typename lambda_t>
    void AddFlow(const internet_header& header, int len, lambda_t callback)
    {
      int flow = FindMatchingFlow(header,len);
      if (flow == max_flows){
        _pkt.header.sys_uptime = htonl(SysUptime());
//        _pkt.header.unix_secs = SysUptime() / 1000;
//        _pkt.header.unix_nsecs = (SysUptime() % 1000) * 1000000;
        if (_pkt.header.count){
          _pkt.header.flow_sequence = htonl(ntohl(_pkt.header.flow_sequence) + 1);
          callback(&_pkt, sizeof(netflow_header) + sizeof(netflow_record) * ntohs(_pkt.header.count));
        }
        _pkt.header.count = 0;
        flow = 0;
      }
      if (flow == ntohs(_pkt.header.count)){
        StartNewFlow(flow, header, len);
        _pkt.header.count = htons(flow + 1);
      }
      else
        AccumulateFlow(flow, header, len);
    }
  };

}
