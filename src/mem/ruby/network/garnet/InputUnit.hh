/*
 * Copyright (c) 2020 Inria
 * Copyright (c) 2016 Georgia Institute of Technology
 * Copyright (c) 2008 Princeton University
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


#ifndef __MEM_RUBY_NETWORK_GARNET_0_INPUTUNIT_HH__
#define __MEM_RUBY_NETWORK_GARNET_0_INPUTUNIT_HH__

#include <iostream>
#include <vector>
////xrwang
#include <queue>

// xrwang
#include <cmath>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include "base/random.hh"
#include <random>
//xrwang

#include "mem/ruby/common/Consumer.hh"
#include "mem/ruby/network/garnet/CommonTypes.hh"
#include "mem/ruby/network/garnet/CreditLink.hh"
#include "mem/ruby/network/garnet/NetworkLink.hh"
#include "mem/ruby/network/garnet/Router.hh"
#include "mem/ruby/network/garnet/VirtualChannel.hh"
#include "mem/ruby/network/garnet/flitBuffer.hh"

#define InputStruct 5
#define AVG_TH 120
#define VAR_TH 10
#define RATE_DEV 0.15
#define BUF_PRO 0.75
#define CALI 1.0 // important to set how many flits to detour //*****11-2 IMPROTANT update, real calibrate
#define TICKC 1000/2
#define CORE_NUM 16
#define THRES_SLOT 100 //for record
#define THRES_FLIT 80 //for record

namespace gem5
{

namespace ruby
{

namespace garnet
{

class InputUnit : public Consumer
{
  public:
    InputUnit(int id, PortDirection direction, Router *router, InputUnitStats& InS0);
    ~InputUnit() = default;

    void wakeup();
    void print(std::ostream& out) const {};

    inline PortDirection get_direction() { return m_direction; }

    inline void
    set_vc_idle(int vc, Tick curTime)
    {
        virtualChannels[vc].set_idle(curTime);
    }

    inline void
    set_vc_active(int vc, Tick curTime)
    {
        virtualChannels[vc].set_active(curTime);
    }

    inline void
    grant_outport(int vc, int outport)
    {
        virtualChannels[vc].set_outport(outport);
    }

    inline void
    grant_outvc(int vc, int outvc)
    {
        virtualChannels[vc].set_outvc(outvc);
    }

    inline int
    get_outport(int invc)
    {
        return virtualChannels[invc].get_outport();
    }

    inline int
    get_outvc(int invc)
    {
        return virtualChannels[invc].get_outvc();
    }

    inline Tick
    get_enqueue_time(int invc)
    {
        return virtualChannels[invc].get_enqueue_time();
    }

    void increment_credit(int in_vc, bool free_signal, Tick curTime);

    inline flit*
    peekTopFlit(int vc)
    {
        return virtualChannels[vc].peekTopFlit();
    }

    inline flit*
    getTopFlit(int vc)
    {
        return virtualChannels[vc].getTopFlit();
    }

    inline bool
    need_stage(int vc, flit_stage stage, Tick time)
    {
        return virtualChannels[vc].need_stage(stage, time);
    }

    inline bool
    isReady(int invc, Tick curTime)
    {
        return virtualChannels[invc].isReady(curTime);
    }

    flitBuffer* getCreditQueue() { return &creditQueue; }

    inline void
    set_in_link(NetworkLink *link)
    {
        m_in_link = link;
    }

    inline int get_inlink_id() { return m_in_link->get_id(); }

    inline void
    set_credit_link(CreditLink *credit_link)
    {
        m_credit_link = credit_link;
    }

    double get_buf_read_activity(unsigned int vnet) const
    { return m_num_buffer_reads[vnet]; }
    double get_buf_write_activity(unsigned int vnet) const
    { return m_num_buffer_writes[vnet]; }

    bool functionalRead(Packet *pkt, WriteMask &mask);
    uint32_t functionalWrite(Packet *pkt);

    void resetStats();

    /////////////////////////////////////////////
    ///////// xrwang
    ////////////////////////////////////////////

    //25-4 revise
    int round_flit = 0;

    struct patternEntry
    {
      int src = 0;
      int dst = 0;
      int inport = 0;
      int output = 0;
      uint64_t cycleRecieve = 0;

      int packet_type = 0; // 0 is 5-flit response and 1 is the 1-flit req

    };

    struct LIH_entry
    {
        int dst = 0;
        uint64_t allocate_time = 0;
        uint64_t update_time = 0;
        uint64_t sampleAvg = 0;
        int packet_type = 0;
    };

    //// the attack variant
    enum Variant{
        Regular = 0,
        Variant_1 = 1,
        Variant_2 = 1<<1,
        Variant_3 = 1<<2,

        ALL = 1+(1<<1)+(1<<2),
    };

    static bool isVariant(int a, Variant vNow)
    {
        if(a == 0)
        {
            return vNow == Variant::Regular;
        }
        else if(a == Variant::ALL)
        {
            return true;
        }
        else if ((a & 0b1) == Variant_1)
        {
            return vNow == Variant::Variant_1;
        }
        else if ((a & 0b10) == Variant_2)
        {
            return vNow == Variant::Variant_2;
        }
        else if ((a & 0b100) == Variant_3)
        {
            return vNow == Variant::Variant_3;
        }

    }



    class HistoryRecord{
        public:
        /// 7-25 this is shor-term injection history S-IH
        std::vector<patternEntry> patternTable;
        int patternLen = 8;

        /// 7-25 this is long-term injection history L-IH
        std::vector<LIH_entry> LIH_table;
        int LIH_length = 4;


        // bool entryCmp(patternEntry& AEntry, patternEntry& BEntry);
        // bool riskFind(patternEntry& riskEntry);

        HistoryRecord(int size)
        {
            patternLen = size;
            for (size_t i = 0; i < patternLen; i++)
            {
                patternTable.push_back(patternEntry());
            }

            for (size_t i = 0; i < LIH_length; i++)
            {
                LIH_table.push_back(LIH_entry());
            }
        }

        void updateHistory(patternEntry riskEntry)
        {
            patternTable.erase(patternTable.begin());
            patternTable.push_back(riskEntry);
        }

        void printTable()
        {
            std::cout<<"==> pattern Table "<<std::endl;
            for (size_t i = 0; i < patternLen; i++)
            {
                std::cout<<patternTable[i].src<<" "<<patternTable[i].dst<<" "<<patternTable[i].inport<<" "<<patternTable[i].output<<" "<<patternTable[i].cycleRecieve<<std::endl;
            }
            std::cout<<std::endl;
        }

        ////////////////////////////////////////////////
        //xrwang for protection
        ///////////////////////////////////////////////

        template<typename ForwardIt>
        inline double ComputeVariance(ForwardIt first, ForwardIt last)
        {
            using ValueType = typename std::iterator_traits<ForwardIt>::value_type;
            auto size = std::distance(first, last);
            if (size <= 1) {
                return 0;
            }

            double avg = std::accumulate(first, last, 0.0) / size;
            double variance(0);
            std::for_each(first, last, [avg, &variance](const ValueType &num) { variance += (num - avg) * (num - avg); });
            return std::sqrt(variance / size);
        }

        inline int ComputeFluctuation(std::vector<uint64_t>& time)
        {
            double avg=0; // = std::accumulate(first, last, 0.0) / size;
            for (size_t i = 0; i < time.size(); i++)
            {
                avg += (time[i]);
            }

            avg = avg/(time.size());
            std::vector<int> repo;
            for(auto i : time)
            {
                repo.push_back(std::abs(i - avg));
            }
            return *(std::max_element(repo.begin(),repo.end()));
        }

        inline double ComputeAvg(std::vector<uint64_t>& time)
        {
            double avg=0; // = std::accumulate(first, last, 0.0) / size;
            for (size_t i = 0; i < time.size()-1; i++)
            {
                avg += (time[i+1] - time[i]);
            }

            return avg/(time.size()-1);
        }


        ////// compare two entries the <src,dst,outport>
        bool
        entryCmp(patternEntry& AEntry, patternEntry& BEntry)
        {
            return AEntry.src == BEntry.src && AEntry.dst == BEntry.dst && AEntry.output == BEntry.output;
        }


        //////////////////////
        // match if the new avg is in the reasonale range
        //////////////////////
        bool compareAvg(int avg0, int avgRecord)
        {
            if (avg0> (avgRecord - avgRecord*0.1) && (avg0 < (avgRecord + avgRecord*0.1)))
            {
                return true;
            }
            else
            {
                return false;
            }

        }

        ///////////////// 8-12 update the risk judgement
        bool
        riskFind_Gen(patternEntry riskEntryNow)
        {
            for (size_t i = 0; i < patternLen; i++)
            {
                if (patternTable[i].cycleRecieve == 0)
                {
                    return false;
                }
            }

            int* sum = new int[patternLen]{0};
            // patternEntry* riskEntry;
            for (int i = 0; i<patternLen; i++)
            {
                for (int j = 0; j < patternLen; j++)
                {
                    bool same = entryCmp(patternTable[i],patternTable[j]);
                    if (same)
                    {
                        sum[i] += 1;
                    }
                }
            }
            //cycle pattern
            patternEntry riskEntry;
            std::vector<uint64_t> timePattern;
            bool find = false;
            for (int i = 0; i<patternLen; i++)
            {
                if (sum[i] >= patternLen*BUF_PRO)
                {
                    riskEntry = patternTable[i];
                    find = true;
                }
            }
            delete [] sum;
            if (!find)
                return false;
            else
            {
                if (!entryCmp(riskEntryNow, riskEntry))
                    return false;
                else
                    return true;
            }

        }

        ///////////////// 7-30, only update L-IH in packet head
        bool
        riskFind(patternEntry riskEntryNow, uint64_t& Pkt_avg, bool packetHead, outportRecord* outBuf, int nowRoute, int destRoute, int variant, InputUnitStats& InfoS, int ourport_preGet)
        {
            for (size_t i = 0; i < patternLen; i++)
            {
                if (patternTable[i].cycleRecieve == 0)
                {
                    return false;
                }
            }

            float threshold = 0.5;
            int* sum = new int[patternLen]{0};
            // patternEntry* riskEntry;
            for (int i = 0; i<patternLen; i++)
            {
                for (int j = 0; j < patternLen; j++)
                {
                    // bool same = patternTable[i].src == patternTable[j].src && patternTable[i].dst == patternTable[j].dst
                                // && patternTable[i].inport == patternTable[j].inport && patternTable[i].output == patternTable[j].output;
                    bool same = entryCmp(patternTable[i],patternTable[j]);
                    if (same)
                    {
                        sum[i] += 1;
                    }
                }
            }

            patternEntry riskEntry;

            //cycle pattern
            std::vector<uint64_t> timePattern;
            bool find = false;
            for (int i = 0; i<patternLen; i++)
            {
                if (sum[i] > patternLen*threshold)
                {
                    riskEntry = patternTable[i];
                    // timePattern.push_back(patternTable[i].cycleRecieve);
                    find = true;
                }
            }
            delete [] sum;
            if (!find && !isVariant(variant, Variant::Variant_3))
                return false;

            //////////////////////
            // this entry is not the dangerous entry, return
            /////////////////////
            if (!entryCmp(riskEntryNow, riskEntry) && !isVariant(variant, Variant::Variant_3))
                return false;

            bool hasRisk = false;
            double avg;
            double variance;

            if(find && entryCmp(riskEntryNow, riskEntry))
            {
                /////////////////// 5.8 need to filter out the "packets" has some regulation
                /////////////////// so we need firstly filter out the same packets, then send to compute module
                for (int i = 0; i<patternLen; i++)
                {
                    if (entryCmp(patternTable[i], riskEntry))
                    {
                        timePattern.push_back(patternTable[i].cycleRecieve);
                    }
                }

                std::vector<uint64_t> timePatternDiff;
                for (size_t i = 0; i < timePattern.size()-1; i++)
                {
                    timePatternDiff.push_back(timePattern[i+1]-timePattern[i]);
                }
                variance = ComputeVariance(timePatternDiff.begin(),timePatternDiff.end());
                avg= ComputeAvg(timePattern);
                if(SecFlag)
                    std::cout<<"The variance: "<<variance<<" avg: "<<avg<<std::endl;


                if(isVariant(variant, Variant::Regular))
                {
                    bool test = variance <= VAR_TH && (std::abs(avg) < AVG_TH);
                    if(test)
                        InfoS.regular_Var ++;
                    hasRisk = test;
                }
                if(isVariant(variant, Variant::Variant_1))
                {
                    bool test = (ComputeFluctuation(timePatternDiff) < avg) && (std::abs(avg) < AVG_TH);
                    if(SecFlag)
                        std::cout<<"fluctuation: "<<ComputeFluctuation(timePatternDiff)<<" avg: "<<avg<<std::endl;
                    if(test)
                    {
                        if(SecFlag)
                            std::cout<<"----> V1  has "<<std::endl;
                        InfoS.Var_1 ++;
                    }
                    hasRisk = test;
                }
                if(isVariant(variant, Variant::Variant_2))
                {
                    bool test = entryCmp(riskEntryNow, riskEntry) && find;
                    if(test)
                    {
                        if(SecFlag)
                            std::cout<<"----> V2  has "<<std::endl;
                        InfoS.Var_2 ++;
                    }
                    hasRisk = test;
                }
            }

            if(isVariant(variant, Variant::Variant_3))
            {
                int num = 0;
                outBuf->findMutual(nowRoute, destRoute, num, ourport_preGet);
                if (num >= (0.5*outBuf->eachEn))
                {
                    hasRisk = true;
                    InfoS.Var_3 ++;
                    if(SecFlag)
                    {
                        std::cout<<"----> V3  has "<<std::endl;
                        outBuf->printRecord();
                    }
                }
                else
                {
                    if(SecFlag)
                        outBuf->printRecord();
                }

            }

            if (hasRisk)
            {
                Pkt_avg = avg;

                ////////// 7-25 here we update the L-HT
                //1. find if there's exsiting one matching entry
                if (packetHead)
                {
                    if(find && entryCmp(riskEntryNow, riskEntry))
                        updateLIH(riskEntryNow, Pkt_avg);
                }
                return true;
            }
            else
                return false;
        }

        /////////////////
        // 8-14 we decouple the updating LIH
        ////////////////
        void updateLIH(patternEntry riskEntryNow, uint64_t Pkt_avg)
        {
            bool hasFind = false;
            uint64_t interval = 1000;
            for (size_t i = 0; i < LIH_length; i++)
            {
                bool match = compareAvg(Pkt_avg, LIH_table[i].sampleAvg) && (riskEntryNow.dst == LIH_table[i].dst);
                if (match)
                {
                    hasFind = true;
                    if ((riskEntryNow.cycleRecieve - LIH_table[i].update_time) <= interval)
                    {
                        LIH_table[i].update_time = riskEntryNow.cycleRecieve;
                    }
                    else
                    {
                        LIH_table[i].update_time = riskEntryNow.cycleRecieve;
                        LIH_table[i].allocate_time = riskEntryNow.cycleRecieve; // reallocate this entry
                    }
                }
            }
            if (!hasFind)
            {
                //update a new entry
                LIH_table.erase(LIH_table.begin());
                LIH_entry updateNew{riskEntryNow.dst,riskEntryNow.cycleRecieve,riskEntryNow.cycleRecieve,Pkt_avg,riskEntryNow.packet_type};
                LIH_table.push_back(updateNew);
            }
        }


        //////////////
        // get the risk level to decide the deflection level
        /////////////

        /// @brief used to judge the risk level
        /// @param existing
        /// @return
        int non_linear_judge(u_int64_t existing)
        {
            int riskLevel = 0;
            if (existing < 5000)
            {
                riskLevel = 4;
            }
            // else if ((existing >= 4000) && (existing <6000))
            // {
            //     riskLevel = 1;
            // }
            // else if ((existing >= 6000) && (existing <7000))
            // {
            //     riskLevel = 2;
            // }
            // else if ((existing >= 7000) && (existing <7500))
            // {
            //     riskLevel = 3;
            // }
            else if (existing >= 5000)
            {
                riskLevel = 4;
            }
            return riskLevel;
        }

        ///////////////////////
        // Get the delay bounds
        //////////////////////
        std::vector<uint64_t> getDelayBounds(int sampleC, bool hasMutual = false)
        {
            std::vector<uint64_t> tmp;
            if (!hasMutual)
            {
                tmp.push_back(sampleC);
                tmp.push_back(0);
            }
            else
            {
                tmp.push_back(sampleC*1.5);
                tmp.push_back(0);
            }
            return tmp;
        }

        /// @brief
        /// @param dst as the name
        /// @param pkg_avg as the name
        /// @param loose_mode to decide the rule of level (1 means not to compare pkg)
        /// @return
        int getRiskLevel(int dst, uint64_t pkg_avg, int loose_mode = 0, uint64_t loose_input_nowTime = 0)
        {
            bool hasFind = false;
            int riskLevel = 0;
            if(SecFlag)
                std::cout<<"in getRiskLevel dst is"<<dst<<std::endl;
            for (size_t i = 0; i < LIH_length; i++)
            {
                bool match = false;
                if (loose_mode == 0)
                {
                    match = (dst == LIH_table[i].dst) && compareAvg(pkg_avg, LIH_table[i].sampleAvg);
                }
                else if (loose_mode == 1){
                    match = (dst == LIH_table[i].dst);
                }
                if(SecFlag)
                    std::cout<<"in getRiskLevel LIH dst is"<<LIH_table[i].dst<<" "<<LIH_table[i].allocate_time<<std::endl;
                if (match)
                {
                    hasFind = true;
                    uint64_t existing=0;
                    if(loose_mode == 0)
                        existing = LIH_table[i].update_time - LIH_table[i].allocate_time;
                    else
                        existing = loose_input_nowTime - LIH_table[i].allocate_time;
                    riskLevel = non_linear_judge(existing);
                    break;
                }
            }
            if(loose_mode == 0)
                assert(hasFind);
            return riskLevel;
        }


    };

    HistoryRecord HR;
    outportRecord* outBuf;

    ////////////////////////
    //// need system time as the seed every time to simulate the TRNG
    ////////////////////////
    class myRandom{
        public:
        std::mt19937 randomG;

        myRandom()
        {
            std::random_device rd;
            randomG = std::mt19937(rd());
        }

        uint64_t getMyRandom(int arg0, int arg1)
        {
            std::uniform_int_distribution<uint64_t> distribution(arg0, arg1);
            return distribution(randomG);
        }
    };
    myRandom mR;

    uint64_t getCycle(uint64_t curTick)
    {
        return curTick / int(TICKC);
    }

    //// for record stats
    InputUnitStats& InS;


  private:
    Router *m_router;
    int m_id;
    PortDirection m_direction;
    int m_vc_per_vnet;
    NetworkLink *m_in_link;
    CreditLink *m_credit_link;
    flitBuffer creditQueue;

    // Input Virtual channels
    std::vector<VirtualChannel> virtualChannels;

    // Statistical variables
    std::vector<double> m_num_buffer_writes;
    std::vector<double> m_num_buffer_reads;
};

} // namespace garnet
} // namespace ruby
} // namespace gem5

#endif // __MEM_RUBY_NETWORK_GARNET_0_INPUTUNIT_HH__
