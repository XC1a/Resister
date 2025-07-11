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


#ifndef __MEM_RUBY_NETWORK_GARNET_0_COMMONTYPES_HH__
#define __MEM_RUBY_NETWORK_GARNET_0_COMMONTYPES_HH__

#include "mem/ruby/common/NetDest.hh"

namespace gem5
{

namespace ruby
{

namespace garnet
{

// All common enums and typedefs go here

enum flit_type {HEAD_, BODY_, TAIL_, HEAD_TAIL_,
                CREDIT_, NUM_FLIT_TYPE_};
enum VC_state_type {IDLE_, VC_AB_, ACTIVE_, NUM_VC_STATE_TYPE_};
enum VNET_type {CTRL_VNET_, DATA_VNET_, NULL_VNET_, NUM_VNET_TYPE_};
enum flit_stage {I_, VA_, SA_, ST_, LT_, NUM_FLIT_STAGE_};
enum link_type { EXT_IN_, EXT_OUT_, INT_, NUM_LINK_TYPES_ };
enum RoutingAlgorithm { TABLE_ = 0, XY_ = 1, RANWESTFIRST_ = 2, SECTEST_ = 3, TEST_ = 4, DEFENSE0_ = 5, DEFENSE1_ = 6, ORIVA = 7, CUSTOM_ = 8,
                        NUM_ROUTING_ALGORITHM_};	// *Taotao Xu*

struct RouteInfo
{
    RouteInfo()
        : vnet(0), src_ni(0), src_router(0), dest_ni(0), dest_router(0),
          hops_traversed(0)
    {}

    // destination format for table-based routing
    int vnet;
    NetDest net_dest;

    // src and dest format for topology-specific routing
    int src_ni;
    int src_router;
    int dest_ni;
    int dest_router;
    int hops_traversed;

    /////////// 24-8-20 for valiant algorithm
    int sTag = 0;
    int inter_route = -1;
    bool passedMid = false;

    void setTag(int newT)
    {
        sTag = newT;
    }

    void setInter(int inter)
    {
        inter_route = inter;
    }

    int getTag()
    {
        return sTag;
    }

    int getInter()
    {
        return inter_route;
    }

    void setPassed()
    {
        passedMid = true;
    }

    bool getPassed()
    {
        return passedMid;
    }
};

#define INFINITE_ 10000



    ////////////// xrwang 24-7-26
    // outport history record, to help the delfection to make decisions
    //////////////
    class outportRecord{
      public:

      std::vector<std::vector<std::tuple<int,int>>> record;
      int numPort = 0;

      int eachEn = 8;

      outportRecord()
      {
        ;
      }

      outportRecord(int size)
      {
        // numPort = route->m_output_unit.size();
        numPort = size;
        assert(size>0);
        for (size_t i = 0; i < numPort; i++)
        {
            record.push_back(std::vector<std::tuple<int,int>>());
            for (size_t j = 0; j < eachEn; j++)
            {
                record[i].push_back(std::make_tuple(-1,-1));
            }
            
        }
        std::cout<<"  Outport record initial with "<<size<<" ports"<<std::endl;
      }

      void printRecord()
      {
        std::cout<<" Print the Record"<<std::endl;
        for (size_t i = 0; i < numPort; i++)
        {
            for (size_t j = 0; j < eachEn; j++)
            {
                std::cout<<"("<<std::get<0>(record[i][j])<<","<<std::get<1>(record[i][j])<<") ";
            }
            std::cout<<std::endl;
            
        }

      }

     void update(int outport, int src_rout, int dest_rout)
     {
        // std::cout<<"  Outport size"<<numPort<<" ourport update "<<outport<<std::endl;
      record[outport].erase(record[outport].begin());
      record[outport].push_back(std::make_tuple(src_rout,dest_rout));

    //   for (size_t i = 0; i < numPort; i++)
    //   {
    //     if (i != outport)
    //     {
    //         record[i].erase(record[i].begin());
    //         record[i].push_back(std::make_tuple(-1,-1));
    //     }
    //   }
     }

      bool findMutual(int srcRoute, int destRoute, int& conNum, int ourport)
      {
        bool findInThis = false;
        for (size_t i = 0; i < eachEn; i++)
        {
            findInThis = (std::get<0>(record[ourport][i]) == srcRoute) && (std::get<1>(record[ourport][i]) == destRoute);
            if (findInThis)
            {
                break;
            }
        }
        if (!findInThis)
        {
            return false;
        }
        

        bool hasM = false;
        if (findInThis)
        {
            for (size_t i = 0; i < eachEn; i++)
            {
                if (std::get<0>(record[ourport][i]) != -1)
                {
                    hasM = (std::get<0>(record[ourport][i]) == srcRoute) && (std::get<1>(record[ourport][i]) == destRoute);
                    if (hasM)
                    {
                        conNum ++;
                    }
                }
            }
        }

        if (conNum!=0)
        {
            return true;
        }

        return false;
      }

    };


} // namespace garnet
} // namespace ruby
} // namespace gem5

#endif //__MEM_RUBY_NETWORK_GARNET_0_COMMONTYPES_HH__
