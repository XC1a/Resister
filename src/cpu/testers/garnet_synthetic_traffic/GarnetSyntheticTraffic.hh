/*
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

#ifndef __CPU_GARNET_SYNTHETIC_TRAFFIC_HH__
#define __CPU_GARNET_SYNTHETIC_TRAFFIC_HH__

#include <set>
#include <vector>	// *Taotao Xu*	append

#include "base/statistics.hh"
#include "mem/port.hh"
#include "params/GarnetSyntheticTraffic.hh"
#include "sim/clocked_object.hh"
#include "sim/eventq.hh"
#include "sim/sim_exit.hh"
#include "sim/sim_object.hh"
#include "sim/stats.hh"

namespace gem5
{

enum TrafficType {BIT_COMPLEMENT_ = 0,
                  BIT_REVERSE_ = 1,
                  BIT_ROTATION_ = 2,
                  NEIGHBOR_ = 3,
                  SHUFFLE_ = 4,
                  TORNADO_ = 5,
                  TRANSPOSE_ = 6,
                  UNIFORM_RANDOM_ = 7,
                  NUM_TRAFFIC_PATTERNS_};

class Packet;
class GarnetSyntheticTraffic : public ClockedObject
{
  public:
    typedef GarnetSyntheticTrafficParams Params;
    GarnetSyntheticTraffic(const Params &p);

    void init() override;

    // main simulation loop (one cycle)
    void tick();

    Port &getPort(const std::string &if_name,
                  PortID idx=InvalidPortID) override;

    /**
     * Print state of address in memory system via PrintReq (for
     * debugging).
     */
    void printAddr(Addr a);

  protected:
    EventFunctionWrapper tickEvent;

    class CpuPort : public RequestPort
    {
        GarnetSyntheticTraffic *tester;

      public:

        CpuPort(const std::string &_name, GarnetSyntheticTraffic *_tester)
            : RequestPort(_name, _tester), tester(_tester)
        { }

      protected:

        virtual bool recvTimingResp(PacketPtr pkt);

        virtual void recvReqRetry();
    };

    CpuPort cachePort;

    class GarnetSyntheticTrafficSenderState : public Packet::SenderState
    {
      public:
        /** Constructor. */
        GarnetSyntheticTrafficSenderState(uint8_t *_data)
            : data(_data)
        { }

        // Hold onto data pointer
        uint8_t *data;
    };

    PacketPtr retryPkt;
    unsigned size;
    int id;

    std::map<std::string, TrafficType> trafficStringToEnum;

    unsigned blockSizeBits;

    Tick noResponseCycles;

    int numDestinations;
    Tick simCycles;
    int numPacketsMax;
    int numPacketsSent;
    int singleSender;
    int singleDest;

    // xrwang
    int packetRule;
    int periodCycle;
    int whiteNoiseDeviation;
    int Deviation_low;
    int DeviationRecord = 0;

    bool T_reverse = true;
    int ptr = 0;
    double timerRule;

    //variant 3
    std::vector<uint64_t> schedule_time; 
    double timerRule_schedule;

    ////// comment by xr wang
    /*
    1.here is key function, we need to control following parameters to gen different patterns
    --core idea--  abstract general common points
    --Thought 001-- func1 and func2 could continue access network for a period of time =>? randomime the func access pattern in a short time
    --Thought 002-- what data access (read or write or control) =>? temporarily using reading
    --Thought 003-- how to design read and write =>? leave to further discuss
                    --Version 01-- always writing to far node
    *LOOPT* total loop time
    *Func1_T* func1 times of accesses far nodes
    *Func1_F* func1 Frequency of accesses far nodes
    *Func1_Itl* func1 interval of accesses far nodes, -1 means random(0,5)
    *Func2_T* func2 times of accesses far nodes
    *Func2_F* func2 Frequency of accesses far nodes
    *Func2_Itl* func1 interval of accesses far nodes, -1 means random(0,5)
    *Loop_Itl* Loop interval between 2 critical caculation
    *_2F_Itl* 2 functions interval
    */

    int LOOPT; // 1
    int Func1_T; // 4
    int Func1_Itl; // 2
    int Func2_T; // 5
    int Func2_Itl; // 2
    int LOOP_Itl; // 10
    int _2F_Itl; // 10

    int debugF; // 10

    std::string fileTrace; // string
    std::ifstream fileToRead;
    std::string lineCotent;

    bool traceDone = false;
    bool waitTiming = false;
    bool read = false;
    uint64_t timeStamp;
    //xrwang

    // *Taotao Xu*
    //int singleSend1;
    //int singleDest1;

    //std::vector<int> multiSender;
    //std::vector<int> multiDest;
    // *Taotao Xu*

    std::string trafficType; // string
    TrafficType traffic; // enum from string
    double injRate;
    int injVnet;
    int precision;

    const Cycles responseLimit;

    RequestorID requestorId;

    void completeRequest(PacketPtr pkt);

    void generatePkt();
    //xr wang
    void generatePkt(int PType, bool chooseDst=false, int choose_dest = 0);
    enum STATE{
      POLLING = 0,
      READ = 1,
      WRITE = 2,
      DONE = 3
    };
    class Smachine{
      public:
      int loopCnt = 0;
      int func1Cnt = 0;
      int func2Cnt = 0;
      Cycles C;
      STATE State = POLLING;
    };
    Smachine Sm;
    //xr wang
    void sendPkt(PacketPtr pkt);
    void initTrafficType();

    void doRetry();

    friend class MemCompleteEvent;
};

} // namespace gem5

#endif // __CPU_GARNET_SYNTHETIC_TRAFFIC_HH__
