/*
 * Copyright (c) 2008 Princeton University
 * Copyright (c) 2016 Georgia Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __MEM_RUBY_NETWORK_GARNET_0_ROUTINGUNIT_HH__
#define __MEM_RUBY_NETWORK_GARNET_0_ROUTINGUNIT_HH__

#include "mem/ruby/common/Consumer.hh"
#include "mem/ruby/common/NetDest.hh"
#include "mem/ruby/network/garnet/CommonTypes.hh"
#include "mem/ruby/network/garnet/GarnetNetwork.hh"
#include "mem/ruby/network/garnet/flit.hh"

namespace gem5
{

namespace ruby
{

namespace garnet
{

class InputUnit;
class Router;

class RoutingUnit
{
  public:
    RoutingUnit(Router *router);
    int outportCompute(RouteInfo& route,
                      int inport,
                      PortDirection inport_dirn);

    // Topology-agnostic Routing Table based routing (default)
    void addRoute(std::vector<NetDest>& routing_table_entry);
    void addWeight(int link_weight);

    // get output port from routing table
    int  lookupRoutingTable(int vnet, NetDest net_dest);

    // Topology-specific direction based routing
    void addInDirection(PortDirection inport_dirn, int inport);
    void addOutDirection(PortDirection outport_dirn, int outport);

    // Routing for Mesh
    int outportComputeXY(RouteInfo route,
                         int inport,
                         PortDirection inport_dirn);

    int outportComputeXY(RouteInfo route,
                         int inport,
                         PortDirection inport_dirn, bool mid);
	// *Taotao Xu*
	// Algorithm 2 west first plus random factor in it!
	int outportRanWestFirst(RouteInfo route,
                         int inport,
                         PortDirection inport_dirn);

	// *Taotao Xu*
	// Algorithm 3 west first plus counter function in it!
	int outportSecTest(RouteInfo route,
                        int inport,
                        PortDirection inport_dirn);

	// *Taotao Xu*
	// Algorithm 4 for adaptive routing random by means of counter function!
	int outportTest(RouteInfo route,
                        int inport,
                        PortDirection inport_dirn);

	// *xrwang*
	// Algorithm 5 for defense algorithm 0
	int outportPattern(RouteInfo route,
                        int inport,
                        PortDirection inport_dirn);

  //secure valiant algorithm
	int outportValiant(RouteInfo& route,
                        int inport,
                        PortDirection inport_dirn);

  //ori valiant algorithm
	int outportOriValiant(RouteInfo& route,
                        int inport,
                        PortDirection inport_dirn);

    // Custom Routing Algorithm using Port Directions
    int outportComputeCustom(RouteInfo route,
                             int inport,
                             PortDirection inport_dirn);

    // Returns true if vnet is present in the vector
    // of vnets or if the vector supports all vnets.
    bool supportsVnet(int vnet, std::vector<int> sVnets);


	PortDirection compute_dirn(int index);	// *Taotao Xu*
    int compute_neighbor_id(int my_id, int index, int num_cols);	// *Taotao Xu*


  private:
    Router *m_router;

    // Routing Table
    std::vector<std::vector<NetDest>> m_routing_table;
    std::vector<int> m_weight_table;

    // Inport and Outport direction to idx maps
    std::map<PortDirection, int> m_inports_dirn2idx;
    std::map<int, PortDirection> m_inports_idx2dirn;
    std::map<int, PortDirection> m_outports_idx2dirn;
    std::map<PortDirection, int> m_outports_dirn2idx;

    // xrwang
    //items for recording the patterns and functions to track the patterns
    struct patternEntry
    {
      int src = 0;
      int dst = 0;
      int inport = 0;
      PortDirection output = 0;
      uint64_t cycleRecieve = 0;

    };

    std::vector<patternEntry> patternTable;
    int patternLen = 8;

    bool entryCmp(patternEntry& AEntry, patternEntry& BEntry);
    bool riskFind(patternEntry& riskEntry);

    void printTable()
    {
      std::cout<<"==> pattern Table "<<std::endl;
      for (size_t i = 0; i < patternLen; i++)
      {
        std::cout<<patternTable[i].src<<" "<<patternTable[i].dst<<" "<<patternTable[i].inport<<" "<<patternTable[i].output<<" "<<patternTable[i].cycleRecieve<<std::endl;
      }
      std::cout<<std::endl;
    }
    // xrwang
};

} // namespace garnet
} // namespace ruby
} // namespace gem5

#endif // __MEM_RUBY_NETWORK_GARNET_0_ROUTINGUNIT_HH__
