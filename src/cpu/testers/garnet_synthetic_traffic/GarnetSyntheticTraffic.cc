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

#include "cpu/testers/garnet_synthetic_traffic/GarnetSyntheticTraffic.hh"

#include <cmath>
#include <ctime>	// *Taotao Xu*
#include <iomanip>
#include <set>
#include <string>
#include <vector>

#include "base/logging.hh"
#include "base/random.hh"
#include "base/statistics.hh"
#include "debug/GarnetSyntheticTraffic.hh"
#include "mem/packet.hh"
#include "mem/port.hh"
#include "mem/request.hh"
#include "sim/sim_events.hh"
#include "sim/stats.hh"
#include "sim/system.hh"

namespace gem5
{

int TESTER_NETWORK=0;

bool
GarnetSyntheticTraffic::CpuPort::recvTimingResp(PacketPtr pkt)
{
    tester->completeRequest(pkt);
    return true;
}

void
GarnetSyntheticTraffic::CpuPort::recvReqRetry()
{
    tester->doRetry();
}

void
GarnetSyntheticTraffic::sendPkt(PacketPtr pkt)
{
    if (!cachePort.sendTimingReq(pkt)) {
        retryPkt = pkt; // RubyPort will retry sending
    }
    numPacketsSent++;
}

GarnetSyntheticTraffic::GarnetSyntheticTraffic(const Params &p)
    : ClockedObject(p),
      tickEvent([this]{ tick(); }, "GarnetSyntheticTraffic tick",
                false, Event::CPU_Tick_Pri),
	  cachePort("GarnetSyntheticTraffic", this),
      retryPkt(NULL),
      size(p.memory_size),
      blockSizeBits(p.block_offset),
      numDestinations(p.num_dest),
      simCycles(p.sim_cycles),
      numPacketsMax(p.num_packets_max),
      numPacketsSent(0),
      singleSender(p.single_sender),
      singleDest(p.single_dest),

      // xrwang
      packetRule(p.packet_rule),
      periodCycle(p.rule_period_cycle),
      whiteNoiseDeviation(p.whiteND),
      Deviation_low(p.whiteND_LOW),

      LOOPT(p.LoopT), // 1
      Func1_T(p.Func1T), // 4
      Func1_Itl(p.Func1Itl), // 1
      Func2_T(p.Func2T), // 5
      Func2_Itl(p.Func2Itl), // 2
      LOOP_Itl(p.LOOPItl), // 10
      _2F_Itl(p.FItl), // 10
      timerRule(p.FItl),

      fileTrace(p.TraceFile),
      //xrwang

      trafficType(p.traffic_type),
      injRate(p.inj_rate),
      injVnet(p.inj_vnet),
      precision(p.precision),
      responseLimit(p.response_limit),
      requestorId(p.system->getRequestorId(this))
{
    // set up counters
    noResponseCycles = 0;
    schedule(tickEvent, 0);

    initTrafficType();
    if (trafficStringToEnum.count(trafficType) == 0) {
        fatal("Unknown Traffic Type: %s!\n", traffic);
    }
    traffic = trafficStringToEnum[trafficType];

    id = TESTER_NETWORK++;
    DPRINTF(GarnetSyntheticTraffic,"Config Created: Name = %s , and id = %d\n",
            name(), id);

    ///// xrwang, and to open this file
    if (packetRule==2)
    {
        fileToRead.open(fileTrace, std::ios::in);
        if (!fileToRead.is_open())
        {
            std::cout<<"Cant find file in "<<fileTrace<<std::endl;
        }
    }
}

Port &
GarnetSyntheticTraffic::getPort(const std::string &if_name, PortID idx)
{
    if (if_name == "test")
        return cachePort;
    else
        return ClockedObject::getPort(if_name, idx);
}

void
GarnetSyntheticTraffic::init()
{
    numPacketsSent = 0;
}


void
GarnetSyntheticTraffic::completeRequest(PacketPtr pkt)
{
    DPRINTF(GarnetSyntheticTraffic,
            "Completed injection of %s packet for address %x\n",
            pkt->isWrite() ? "write" : "read\n",
            pkt->req->getPaddr());

    assert(pkt->isResponse());
    noResponseCycles = 0;
    delete pkt;
}


void
GarnetSyntheticTraffic::tick()
{
    // SecFlag = true;
    /////////////11-17
    if(curCycle()>=_2F_Itl )
    {

    if (++noResponseCycles >= responseLimit) {
        fatal("%s deadlocked at cycle %d\n", name(), curTick());
    }

    // make new request based on injection rate
    // (injection rate's range depends on precision)
    // - generate a random number between 0 and 10^precision
    // - send pkt if this number is < injRate*(10^precision)
    bool sendAllowedThisCycle;
    //xrwang
    if (packetRule == -1)
    {
    //xrwang
    double injRange = pow((double) 10, (double) precision);
    unsigned trySending = random_mt.random<unsigned>(0, (int) injRange);
    if (trySending < injRate*injRange)
        sendAllowedThisCycle = true;
    else
        sendAllowedThisCycle = false;

    }//xrwang
    else if (packetRule == 0)
    {
        LatDelFlag = true;
        //this is first custome packet generating rules: given send is "1", not send is "0", T is the cycle
        // 0 -T- 1 -T- 0 -T-1 ...

        //we first assume T is 5 cycles
        if ((curCycle() - timerRule) == (periodCycle+DeviationRecord) || T_reverse)
        {
            std::random_device rd;
            std::mt19937 randomG = std::mt19937(rd());

            //// we change from using white noise to using a uniform generator
            // std::normal_distribution<double> dist(0, whiteNoiseDeviation);
            int threLow=Deviation_low;
            std::uniform_int_distribution<int> dist(-whiteNoiseDeviation, whiteNoiseDeviation);
            DeviationRecord = dist(randomG);

            if (DeviationRecord < 0)
                DeviationRecord = DeviationRecord - threLow;
            else
                DeviationRecord = DeviationRecord + threLow;

            if(SecFlag)
                std::cout<<"---- Deviation now is "<<DeviationRecord<<std::endl;
            if (std::abs(DeviationRecord) >= periodCycle)
            {
                DeviationRecord = 0;
            }

            timerRule = curCycle();
            sendAllowedThisCycle = true;
            T_reverse = false;
        }
        else
            sendAllowedThisCycle = false;
    }
    //here we need a state machine to process
    //now begin State machine algorithm 1
    else if (packetRule == 1)
    {
        LatDelFlag = true;
        uint64_t DiffT = curCycle() - Sm.C;
        if(Sm.loopCnt == LOOPT)
        {
            //ending
            Sm.State = STATE::DONE;
        }
        else if(Sm.State == STATE::POLLING)
        {
            //POLLING -> FUNC1(n accesses) -> FUNC2(n access) -> POLLING
            if (Sm.func1Cnt < Func1_T)
            {
                if (DiffT >= Func1_Itl)
                {
                    /* code */
                    Sm.C = curCycle();
                    // generatePkt(2);
                    Sm.func1Cnt ++;
                    Sm.State = STATE::WRITE;
                }
            }
            else if (Sm.func1Cnt == Func1_T && Sm.func2Cnt < Func2_T)
            {
                if (Sm.func2Cnt == 0)
                {
                    if (DiffT >= _2F_Itl)
                    {
                        Sm.C = curCycle();
                        // generatePkt(2);
                        Sm.func2Cnt ++;
                        Sm.State = STATE::WRITE;
                    }
                }
                else
                {
                    if (DiffT >= Func2_Itl)
                    {
                        Sm.C = curCycle();
                        // generatePkt(2);
                        Sm.func2Cnt ++;
                        Sm.State = STATE::WRITE;
                    }
                }
            }
            else if (Sm.func1Cnt == Func1_T && Sm.func2Cnt == Func2_T)
            {
                if (Sm.loopCnt == LOOPT - 1)
                {
                    Sm.State = STATE::DONE;
                }
                else if (DiffT >= LOOP_Itl)
                {
                    Sm.func1Cnt = 0;
                    Sm.func2Cnt = 0;
                    Sm.loopCnt ++;

                    Sm.C = curCycle();
                    // generatePkt(2);
                    Sm.func1Cnt ++;
                    Sm.State = STATE::WRITE;
                }
            }
        }


        if (Sm.State == STATE::READ || Sm.State == STATE::WRITE)
        {
            bool senderEnable = true;
            if (numPacketsMax >= 0 && numPacketsSent >= numPacketsMax)
                senderEnable = false;
            if (singleSender >= 0 && id != singleSender)
                senderEnable = false;
            if (senderEnable)
            {
                generatePkt(2);
                Sm.State = STATE::POLLING;
            }
        }

        if (curTick() >= simCycles)
            exitSimLoop("Network Tester completed simCycles");
        else {
            if (!tickEvent.scheduled())
                schedule(tickEvent, clockEdge(Cycles(1)));
        }
        return;
    }
    else if(packetRule == 2)
    {
        LatDelFlag = true;
        ////////////////// update in 6.24
        ////////////////// Here, we read the trace from a file, then simulate the trace to send
        if (!traceDone)
        {
            if (!waitTiming)
            {
                if (!std::getline(fileToRead,lineCotent))
                {
                    traceDone == true;
                }
                else
                {
                    read = lineCotent[0] == 'R';
                    std::string timeStampS = lineCotent.substr(1,lineCotent.length()-1);
                    timeStamp = stol(timeStampS) + _2F_Itl;
                    if (curCycle() > timeStamp)
                    {
                        // send no
                        assert (!(numPacketsMax >= 0 && numPacketsSent >= numPacketsMax));
                        assert (!(singleSender >= 0 && id != singleSender));
                        waitTiming = false;
                        if (read)
                            generatePkt(1);
                        else
                            generatePkt(2);
                    }
                    else
                    {
                        waitTiming = true;
                        if (!tickEvent.scheduled())
                            schedule(tickEvent, clockEdge(Cycles(1)));
                    }
                }
            }
            else
            {
                if (curCycle() > timeStamp)
                {
                    // send no
                    assert (!(numPacketsMax >= 0 && numPacketsSent >= numPacketsMax));
                    assert (!(singleSender >= 0 && id != singleSender));
                    waitTiming = false;
                    if (read)
                        generatePkt(1);
                    else
                        generatePkt(2);
                }
                else
                {
                    if (!tickEvent.scheduled())
                        schedule(tickEvent, clockEdge(Cycles(1)));
                }
            }
        }
    }
    //// attack sampling variant
    else if (packetRule == 3)
    {
        LatDelFlag = true;
        int diff[] = {20, 30, 40, 50, 60, 70, 80};
        // int diff[] = {30, 40, 50, 60, 70, 80, 90};
        int diff_len = 7;

        //we first assume T is 5 cycles
        periodCycle = diff[ptr];
        if ((curCycle() - timerRule) == (periodCycle) || T_reverse)
        {
            if(SecFlag)
                std::cout<<"---- Deviation now is "<<curCycle()<<std::endl;
            timerRule = curCycle();
            sendAllowedThisCycle = true;
            if (T_reverse)
            {
                ptr = 0;
            }
            else if (ptr < (diff_len-1))
            {
                ++ ptr;
            }
            else{
                ptr = 0;
            }
            T_reverse = false;

        }
        else
            sendAllowedThisCycle = false;
    }
    else if (packetRule == 4)
    {
        LatDelFlag = true;
        int dest_options[] = {4, 5, 7, 8, 9, 11, 12, 13, 15};
        int diff_len = 9;

        if ((curCycle() - timerRule) == (periodCycle) || T_reverse)
        {
            timerRule = curCycle();
            sendAllowedThisCycle = true;
            T_reverse = false;

            schedule_time.clear();
            ptr = 0;

            std::random_device rd;
            if (!(numPacketsMax >= 0 && numPacketsSent >= numPacketsMax))
            {
                /*
                uint64_t arr[2];
                for (int i = 0; i < 2; i++) {
                    srand(rd());
                    arr[i] = ((rand() % (periodCycle-1)) +1 );
                    std::cout<<arr[i]<<" ";
                }
                std::cout<<std::endl;
                std::sort(arr, arr+2);
                for (int i = 0; i < 2; i++) {
                    std::cout<<arr[i]<<" ";
                    // schedule_time.push_back(arr[i]);
                }
                */
                schedule_time.push_back(35);
                schedule_time.push_back(35);
                std::cout<<std::endl;
            }
        }
        // else if (((curCycle() - timerRule) == (schedule_time[0]) && ptr == 0) || (curCycle() - timerRule_schedule) == (schedule_time[ptr]))
        else if((schedule_time.size() != 0) && (((curCycle() - timerRule) == (schedule_time[0]) && ptr == 0) || ((curCycle() - timerRule_schedule) == (schedule_time[ptr])) ))
        {
            std::random_device rd;
            srand(rd());
            int dest_schedule = dest_options[rand() % diff_len];
            if (ptr < (1))
                ++ ptr;
            else
            {
                schedule_time.clear();
                ptr = 0;
            }
            // std::cout<<"  Some packets now ptr is "<<ptr<<std::endl;

            timerRule_schedule = curCycle();
            generatePkt(2,true,dest_schedule);
            // std::cout<<"  Some packets "<<curCycle()<<std::endl;
            if (!tickEvent.scheduled())
                schedule(tickEvent, clockEdge(Cycles(1)));
            return;
        }
        else if((schedule_time.size() != 0) && (ptr != 0))
        {
            // std::cout<<"  wat packets "<<curCycle()<<" timerule_schedule "<<timerRule_schedule<<" point "<<schedule_time[ptr]<<std::endl;
            if (!tickEvent.scheduled())
                schedule(tickEvent, clockEdge(Cycles(1)));
            return;
        }
        else
            sendAllowedThisCycle = false;
    }



    // always generatePkt unless fixedPkts or singleSender is enabled
    if (sendAllowedThisCycle) {
        bool senderEnable = true;

        if (numPacketsMax >= 0 && numPacketsSent >= numPacketsMax)
            senderEnable = false;

        if (singleSender >= 0 && id != singleSender)
            senderEnable = false;

		if (senderEnable)
        {
            if (packetRule == 0 || (packetRule == 3) || (packetRule == 4))
            {
                generatePkt(2);
            }
            else
            {
                generatePkt();
            }
        }
        //xrwang
        if(senderEnable && SecFlag)
            std::cout<<"xrwang stdout ==>> SendNow: "<<id<<" curCycle: "<<curCycle()<<std::endl;
        //wxrwang
    }

    /////////11-17
    }

    // Schedule wakeup
    if (curTick() >= simCycles)
        exitSimLoop("Network Tester completed simCycles");
    else {
        if (!tickEvent.scheduled())
            schedule(tickEvent, clockEdge(Cycles(1)));
    }
}

void
GarnetSyntheticTraffic::generatePkt(int PType, bool chooseDst, int choose_dest)
{
    int num_destinations = numDestinations;
    int radix = (int) sqrt(num_destinations);
    unsigned destination = id;
    int dest_x = -1;
    int dest_y = -1;
    int source = id;
    int src_x = id%radix;
    int src_y = id/radix;

    if (singleDest >= 0)
    {
        destination = singleDest;
    }
    else if (traffic == UNIFORM_RANDOM_) {
    	destination = random_mt.random<unsigned>(0, num_destinations - 1);
    } else if (traffic == BIT_COMPLEMENT_) {
        dest_x = radix - src_x - 1;
        dest_y = radix - src_y - 1;
        destination = dest_y*radix + dest_x;
    } else if (traffic == BIT_REVERSE_) {
        unsigned int straight = source;
        unsigned int reverse = source & 1; // LSB

        int num_bits = (int) log2(num_destinations);

        for (int i = 1; i < num_bits; i++)
        {
            reverse <<= 1;
            straight >>= 1;
            reverse |= (straight & 1); // LSB
        }
        destination = reverse;
    } else if (traffic == BIT_ROTATION_) {
        if (source%2 == 0)
            destination = source/2;
        else // (source%2 == 1)
            destination = ((source/2) + (num_destinations/2));
    } else if (traffic == NEIGHBOR_) {
            dest_x = (src_x + 1) % radix;
            dest_y = src_y;
            destination = dest_y*radix + dest_x;
    } else if (traffic == SHUFFLE_) {
        if (source < num_destinations/2)
            destination = source*2;
        else
            destination = (source*2 - num_destinations + 1);
    } else if (traffic == TRANSPOSE_) {
            dest_x = src_y;
            dest_y = src_x;
            destination = dest_y*radix + dest_x;
    } else if (traffic == TORNADO_) {
        dest_x = (src_x + (int) ceil(radix/2) - 1) % radix;
        dest_y = src_y;
        destination = dest_y*radix + dest_x;
    }
    else {
        fatal("Unknown Traffic Type: %s!\n", traffic);
    }

    Addr paddr;
    if(!chooseDst)
        paddr =  destination;
    else
        paddr =  choose_dest;

    paddr <<= blockSizeBits;
    unsigned access_size = 1; // Does not affect Ruby simulation
    MemCmd::Command requestType;
    RequestPtr req = nullptr;
    Request::Flags flags;


    if (PType == 0) {
        // generate packet for virtual network 0
        requestType = MemCmd::ReadReq;
        req = std::make_shared<Request>(paddr, access_size, flags,
                                        requestorId);
    } else if (PType == 1) {
        // generate packet for virtual network 1
        requestType = MemCmd::ReadReq;
        flags.set(Request::INST_FETCH);
        req = std::make_shared<Request>(
            0x0, access_size, flags, requestorId, 0x0, 0);
        req->setPaddr(paddr);
    } else if (PType == 2) {  // if (injReqType == 2)
        // generate packet for virtual network 2
        requestType = MemCmd::WriteReq;
        req = std::make_shared<Request>(paddr, access_size, flags,
                                        requestorId);
    }

    req->setContext(id);
    PacketPtr pkt = new Packet(req, requestType);
    pkt->dataDynamic(new uint8_t[req->getSize()]);
    pkt->senderState = NULL;
    sendPkt(pkt);

    std::cout<<"CPU S "<<id<<" SEND now "<<curCycle()<<std::endl;
}


void
GarnetSyntheticTraffic::generatePkt()
{
    int num_destinations = numDestinations;
    int radix = (int) sqrt(num_destinations);
    unsigned destination = id;
    int dest_x = -1;
    int dest_y = -1;
    int source = id;
    int src_x = id%radix;
    int src_y = id/radix;

    if (singleDest >= 0)
    {
        destination = singleDest;
    }


    else if (traffic == UNIFORM_RANDOM_) {
    	destination = random_mt.random<unsigned>(0, num_destinations - 1);
    } else if (traffic == BIT_COMPLEMENT_) {
        dest_x = radix - src_x - 1;
        dest_y = radix - src_y - 1;
        destination = dest_y*radix + dest_x;
    } else if (traffic == BIT_REVERSE_) {
        unsigned int straight = source;
        unsigned int reverse = source & 1; // LSB

        int num_bits = (int) log2(num_destinations);

        for (int i = 1; i < num_bits; i++)
        {
            reverse <<= 1;
            straight >>= 1;
            reverse |= (straight & 1); // LSB
        }
        destination = reverse;
    } else if (traffic == BIT_ROTATION_) {
        if (source%2 == 0)
            destination = source/2;
        else // (source%2 == 1)
            destination = ((source/2) + (num_destinations/2));
    } else if (traffic == NEIGHBOR_) {
            dest_x = (src_x + 1) % radix;
            dest_y = src_y;
            destination = dest_y*radix + dest_x;
    } else if (traffic == SHUFFLE_) {
        if (source < num_destinations/2)
            destination = source*2;
        else
            destination = (source*2 - num_destinations + 1);
    } else if (traffic == TRANSPOSE_) {
            dest_x = src_y;
            dest_y = src_x;
            destination = dest_y*radix + dest_x;
    } else if (traffic == TORNADO_) {
        dest_x = (src_x + (int) ceil(radix/2) - 1) % radix;
        dest_y = src_y;
        destination = dest_y*radix + dest_x;
    }
    else {
        fatal("Unknown Traffic Type: %s!\n", traffic);
    }

    // The source of the packets is a cache.
    // The destination of the packets is a directory.
    // The destination bits are embedded in the address after byte-offset.
    Addr paddr =  destination;
    paddr <<= blockSizeBits;
    unsigned access_size = 1; // Does not affect Ruby simulation

    // Modeling different coherence msg types over different msg classes.
    //
    // GarnetSyntheticTraffic assumes the Garnet_standalone coherence protocol
    // which models three message classes/virtual networks.
    // These are: request, forward, response.
    // requests and forwards are "control" packets (typically 8 bytes),
    // while responses are "data" packets (typically 72 bytes).
    //
    // Life of a packet from the tester into the network:
    // (1) This function generatePkt() generates packets of one of the
    //     following 3 types (randomly) : ReadReq, INST_FETCH, WriteReq
    // (2) mem/ruby/system/RubyPort.cc converts these to RubyRequestType_LD,
    //     RubyRequestType_IFETCH, RubyRequestType_ST respectively
    // (3) mem/ruby/system/Sequencer.cc sends these to the cache controllers
    //     in the coherence protocol.
    // (4) Network_test-cache.sm tags RubyRequestType:LD,
    //     RubyRequestType:IFETCH and RubyRequestType:ST as
    //     Request, Forward, and Response events respectively;
    //     and injects them into virtual networks 0, 1 and 2 respectively.
    //     It immediately calls back the sequencer.
    // (5) The packet traverses the network (simple/garnet) and reaches its
    //     destination (Directory), and network stats are updated.
    // (6) Network_test-dir.sm simply drops the packet.
    //
    MemCmd::Command requestType;

    RequestPtr req = nullptr;
    Request::Flags flags;

    // Inject in specific Vnet
    // Vnet 0 and 1 are for control packets (1-flit)
    // Vnet 2 is for data packets (5-flit)
    int injReqType = injVnet;

    if (injReqType < 0 || injReqType > 2)
    {
        // randomly inject in any vnet
        injReqType = random_mt.random(0, 2);
    }

    if (injReqType == 0) {
        // generate packet for virtual network 0
        requestType = MemCmd::ReadReq;
        req = std::make_shared<Request>(paddr, access_size, flags,
                                        requestorId);
    } else if (injReqType == 1) {
        // generate packet for virtual network 1
        requestType = MemCmd::ReadReq;
        flags.set(Request::INST_FETCH);
        req = std::make_shared<Request>(
            0x0, access_size, flags, requestorId, 0x0, 0);
        req->setPaddr(paddr);
    } else {  // if (injReqType == 2)
        // generate packet for virtual network 2
        requestType = MemCmd::WriteReq;
        req = std::make_shared<Request>(paddr, access_size, flags,
                                        requestorId);
    }

    req->setContext(id);

    //No need to do functional simulation
    //We just do timing simulation of the network

    DPRINTF(GarnetSyntheticTraffic,
            "Generated packet with destination %d, embedded in address %x\n",
            destination, req->getPaddr());

    PacketPtr pkt = new Packet(req, requestType);
    pkt->dataDynamic(new uint8_t[req->getSize()]);
    pkt->senderState = NULL;

    sendPkt(pkt);
}

void
GarnetSyntheticTraffic::initTrafficType()
{
    trafficStringToEnum["bit_complement"] = BIT_COMPLEMENT_;
    trafficStringToEnum["bit_reverse"] = BIT_REVERSE_;
    trafficStringToEnum["bit_rotation"] = BIT_ROTATION_;
    trafficStringToEnum["neighbor"] = NEIGHBOR_;
    trafficStringToEnum["shuffle"] = SHUFFLE_;
    trafficStringToEnum["tornado"] = TORNADO_;
    trafficStringToEnum["transpose"] = TRANSPOSE_;
    trafficStringToEnum["uniform_random"] = UNIFORM_RANDOM_;
}

void
GarnetSyntheticTraffic::doRetry()
{
    if (cachePort.sendTimingReq(retryPkt)) {
        retryPkt = NULL;
    }
}

void
GarnetSyntheticTraffic::printAddr(Addr a)
{
    cachePort.printAddr(a);
}

} // namespace gem5
