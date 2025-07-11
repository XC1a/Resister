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


#include "mem/ruby/network/garnet/InputUnit.hh"

#include "debug/RubyNetwork.hh"
#include "mem/ruby/network/garnet/Credit.hh"
#include "mem/ruby/network/garnet/Router.hh"

namespace gem5
{

namespace ruby
{

namespace garnet
{

InputUnit::InputUnit(int id, PortDirection direction, Router *router, InputUnitStats& InS0)
  : Consumer(router), m_router(router), m_id(id), m_direction(direction),
    m_vc_per_vnet(m_router->get_vc_per_vnet()),
    ////////////xrwang data structures
    HR(8),
    mR(),
    InS(InS0)
{
    const int m_num_vcs = m_router->get_num_vcs();
    m_num_buffer_reads.resize(m_num_vcs/m_vc_per_vnet);
    m_num_buffer_writes.resize(m_num_vcs/m_vc_per_vnet);
    for (int i = 0; i < m_num_buffer_reads.size(); i++) {
        m_num_buffer_reads[i] = 0;
        m_num_buffer_writes[i] = 0;
    }

    // Instantiating the virtual channels
    virtualChannels.reserve(m_num_vcs);
    for (int i=0; i < m_num_vcs; i++) {
        virtualChannels.emplace_back();
    }

    ///////////// xrwang 7-26 ourput record
    outBuf = &(router->outBuf);
}

/*
 * The InputUnit wakeup function reads the input flit from its input link.
 * Each flit arrives with an input VC.
 * For HEAD/HEAD_TAIL flits, performs route computation,
 * and updates route in the input VC.
 * The flit is buffered for (m_latency - 1) cycles in the input VC
 * and marked as valid for SwitchAllocation starting that cycle.
 *
 */
#if InputStruct == 0
void
InputUnit::wakeup()
{
    flit *t_flit;
    if (m_in_link->isReady(curTick())) {

        t_flit = m_in_link->consumeLink();
        DPRINTF(RubyNetwork, "Router[%d] Consuming:%s Width: %d Flit:%s\n",
        m_router->get_id(), m_in_link->name(),
        m_router->getBitWidth(), *t_flit);
        assert(t_flit->m_width == m_router->getBitWidth());
        int vc = t_flit->get_vc();
        t_flit->increment_hops(); // for stats

        if ((t_flit->get_type() == HEAD_) ||
            (t_flit->get_type() == HEAD_TAIL_)) {

            assert(virtualChannels[vc].get_state() == IDLE_);
            set_vc_active(vc, curTick());

            // Route computation for this vc
            int outport = m_router->route_compute(t_flit->get_route(0),
                m_id, m_direction);

            // Update output port in VC
            // All flits in this packet will use this output port
            // The output port field in the flit is updated after it wins SA
            grant_outport(vc, outport);

        } else {
            assert(virtualChannels[vc].get_state() == ACTIVE_);
        }

        if (SecFlag)
        {
            std::cout<<"[NowID "<<m_router->get_id()<<" InputUint ID: "<<m_id<<" ]";
            t_flit->print(std::cout);
            std::cout<<std::endl;
        }


        // Buffer the flit
        virtualChannels[vc].insertFlit(t_flit);

        int vnet = vc/m_vc_per_vnet;
        // number of writes same as reads
        // any flit that is written will be read only once
        m_num_buffer_writes[vnet]++;
        m_num_buffer_reads[vnet]++;

        Cycles pipe_stages = m_router->get_pipe_stages();
        if (pipe_stages == 1) {
            // 1-cycle router
            // Flit goes for SA directly
            t_flit->advance_stage(SA_, curTick());
        } else {
            assert(pipe_stages > 1);
            // Router delay is modeled by making flit wait in buffer for
            // (pipe_stages cycles - 1) cycles before going for SA

            Cycles wait_time = pipe_stages - Cycles(1);
            t_flit->advance_stage(SA_, m_router->clockEdge(wait_time));

            // Wakeup the router in that cycle to perform SA
            m_router->schedule_wakeup(Cycles(wait_time));
        }

        if (m_in_link->isReady(curTick())) {
            m_router->schedule_wakeup(Cycles(1));
        }
    }
}

#elif InputStruct == 1 //deprecated

// resister, [latency] is used to obtain the detection latency
#elif InputStruct == 2 || InputStruct == 5 
void
InputUnit::wakeup()
{
    flit *t_flit;
    if (m_in_link->isReady(curTick())) {

        t_flit = m_in_link->consumeLink();
        assert(t_flit->m_width == m_router->getBitWidth());
        if(SecFlag)
        {
            std::cout<<"[NowID "<<m_router->get_id()<<" InputUint ID: "<<m_id<<" ]";
            t_flit->print(std::cout);
            std::cout<<std::endl;
        }
        bool hasRisk=false;
        bool hasHeadRisk=false;
        if (m_id == 0)
        {
            bool headPacket = (t_flit->get_type() == HEAD_) || (t_flit->get_type() == HEAD_TAIL_);
            bool tailPacket = (t_flit->get_type() == TAIL_) || (t_flit->get_type() == HEAD_TAIL_);
            uint64_t pkt_avg = 0;
            if ( headPacket && (t_flit->get_route().getTag() == 0))
            {
                InS.all_counts ++;
                HR.updateHistory({t_flit->get_route().src_router,
                                    t_flit->get_route().dest_router,
                                    m_id,
                                    t_flit->get_route().dest_router,
                                    getCycle(curTick())});
                /*
                hasHeadRisk = HR.riskFind({t_flit->get_route().src_router,
                                t_flit->get_route().dest_router,
                                m_id, // inport
                                t_flit->get_route().dest_router,
                                getCycle(curTick())},
                                pkt_avg, true,
                                outBuf,
                                t_flit->get_route().src_router,
                                t_flit->get_route().dest_router,
                                Variant::Variant_3,
                                InS);
                */
                if(SecFlag)
                {
                    std::cout<<"Now Input Router is "<<m_router->get_id()<<" ";
                    HR.printTable();
                }
            }

            bool thisTo = false;
            double riskLevel=0;
            bool hasMutual = false;

            ////get the contention number to decide the value to delay
            int numContenB = 0;
            float calibrate = 1;

            if (headPacket && (t_flit->get_route().dest_router != m_router->get_id()))
            {
                //fake cal ourport
                int outport_preGet;
                outport_preGet = m_router->route_compute(t_flit->get_route(0),
                    m_id, m_direction);

                hasRisk = HR.riskFind({t_flit->get_route().src_router,
                                    t_flit->get_route().dest_router,
                                    m_id, // inport
                                    t_flit->get_route().dest_router,
                                    getCycle(curTick())},
                                    pkt_avg, true,
                                    outBuf,
                                    t_flit->get_route().src_router,
                                    t_flit->get_route().dest_router,
                                    Variant::ALL,
                                    InS,
                                    outport_preGet);
                if (hasRisk)
                    riskLevel = 4;
                    // riskLevel = HR.getRiskLevel(t_flit->get_route().dest_router, pkt_avg);
                uint64_t injRange = pow((double) 10, (double) 4);
                // unsigned trySending = random_mt.random<unsigned>(0, (int) injRange);
                uint64_t trySending = mR.getMyRandom(0, injRange);
                if (hasRisk) /// only has risk, needs to further get the risk level
                {
                    #if InputStruct == 5
                    uint64_t nowCycle = getCycle(curTick());
                    std::cout<<"Detected "<<nowCycle<<std::endl;
                    exit(0);
                    #endif
                    InS.risk_counts ++;
                    calibrate = CALI;
                    if (trySending < (injRange * calibrate ))
                        thisTo = true;
                }
            }

            if ( headPacket && (t_flit->get_route().dest_router != m_router->get_id()) && hasRisk)
            {
                if (thisTo)
                {
                    InS.detour_counts ++;
                    t_flit->get_route(0).setTag(1);
                    if(SecFlag)
                        std::cout<<"** set tag to 1 real: "<<t_flit->get_route().getTag()<<std::endl;
                }
            }
        }

        int vc = t_flit->get_vc();
        t_flit->increment_hops(); // for stats
        int outport;
        if ((t_flit->get_type() == HEAD_) ||
            (t_flit->get_type() == HEAD_TAIL_)) {

            assert(virtualChannels[vc].get_state() == IDLE_);
            set_vc_active(vc, curTick());
            outport = m_router->route_compute(t_flit->get_route(0),
                m_id, m_direction);
            grant_outport(vc, outport);
            if(m_id == 0)
                outBuf->update(outport, t_flit->get_route().src_router, t_flit->get_route().dest_router);

        } else {
            assert(virtualChannels[vc].get_state() == ACTIVE_);
        }
        virtualChannels[vc].insertFlit(t_flit);
        int vnet = vc/m_vc_per_vnet;
        m_num_buffer_writes[vnet]++;
        m_num_buffer_reads[vnet]++;
        Cycles pipe_stages = m_router->get_pipe_stages();

        if (pipe_stages == 1) {
            t_flit->advance_stage(SA_, curTick());
        } else {
            assert(pipe_stages > 1);
            Cycles wait_time = pipe_stages - Cycles(1);
            t_flit->advance_stage(SA_, m_router->clockEdge(wait_time));

            m_router->schedule_wakeup(Cycles(wait_time));
        }
        if (m_in_link->isReady(curTick())) {
            m_router->schedule_wakeup(Cycles(1));
        }
    }
}

#elif InputStruct == 3 // ori-valliant
void
InputUnit::wakeup()
{
    flit *t_flit;
    if (m_in_link->isReady(curTick())) {

        t_flit = m_in_link->consumeLink();
        assert(t_flit->m_width == m_router->getBitWidth());
        bool hasHeadRisk=false;
        if (m_id == 0)
        {
            bool headPacket = (t_flit->get_type() == HEAD_) || (t_flit->get_type() == HEAD_TAIL_);

            if ( headPacket && (t_flit->get_route().dest_router != m_router->get_id()))
            {
                InS.detour_counts ++;
                t_flit->get_route(0).setTag(1);
                if(SecFlag)
                    std::cout<<"** set tag to 1 real: "<<t_flit->get_route().getTag()<<std::endl;
            }
        }

        int vc = t_flit->get_vc();
        t_flit->increment_hops(); // for stats
        int outport;
        if ((t_flit->get_type() == HEAD_) ||
            (t_flit->get_type() == HEAD_TAIL_)) {

            assert(virtualChannels[vc].get_state() == IDLE_);
            set_vc_active(vc, curTick());
            outport = m_router->route_compute(t_flit->get_route(0),
                m_id, m_direction);
            grant_outport(vc, outport);
        } else {
            assert(virtualChannels[vc].get_state() == ACTIVE_);
        }
        virtualChannels[vc].insertFlit(t_flit);
        int vnet = vc/m_vc_per_vnet;
        m_num_buffer_writes[vnet]++;
        m_num_buffer_reads[vnet]++;
        Cycles pipe_stages = m_router->get_pipe_stages();

        if (pipe_stages == 1) {
            t_flit->advance_stage(SA_, curTick());
        } else {
            assert(pipe_stages > 1);
            Cycles wait_time = pipe_stages - Cycles(1);
            t_flit->advance_stage(SA_, m_router->clockEdge(wait_time));

            m_router->schedule_wakeup(Cycles(wait_time));
        }
        if (m_in_link->isReady(curTick())) {
            m_router->schedule_wakeup(Cycles(1));
        }
    }
}

#elif InputStruct == 4

bool init = false;
int counter[CORE_NUM] = {-1};

int round_cnt = 0;
int round_list[CORE_NUM] = {-1};


void
InputUnit::wakeup()
{
    if (m_id ==0)
    {

        int routerID = m_router->get_id();
        if (!init)
        {
            std::cout<<"init from counter "<<routerID<<std::endl;
            round_cnt = routerID;
            for (size_t i = 0; i < CORE_NUM; i++)
            {
                round_list[i] = i;
                if(i==routerID)
                {
                    counter[i] = 0;
                }
                else
                {
                    counter[i] = -1;
                }
                std::cout<<" counter "<<i<<" "<<counter[i]<<std::endl;;
            }
            init = true;
        }

        int formerID;
        // std::cout<<" router ID is "<<routerID<<std::endl;
        if (routerID > 0)
        {
            formerID = routerID - 1;
        }
        else if(routerID == 0)
        {
            formerID = CORE_NUM-1;
        }
        bool lastOneStart = counter[formerID] != -1;


        bool enable = counter[routerID]>=0;
        bool roundChange = false;
        if (enable)
        {
            //update the counter
            // counter[routerID] ++;
            if (counter[routerID] >= THRES_SLOT )
            {
                roundChange = true;
            }

            if (m_in_link->isReady(curTick()) && !roundChange) {
                // std::cout<<" router ID is "<<routerID<<" counter is "<<counter[routerID]<<std::endl;

                if (round_flit < THRES_FLIT)
                {
                    flit *t_flit;
                    t_flit = m_in_link->consumeLink();

                    bool headPacket = (t_flit->get_type() == HEAD_) || (t_flit->get_type() == HEAD_TAIL_);
                    bool tailPacket = (t_flit->get_type() == TAIL_) || (t_flit->get_type() == HEAD_TAIL_);
                    round_flit ++;

                    assert(t_flit->m_width == m_router->getBitWidth());
                    bool hasHeadRisk=false;
                    if(t_flit->get_route().dest_router != m_router->get_id())
                    {
                        if (tailPacket){
                            InS.detour_counts ++;
                        }
                    }

                    int vc = t_flit->get_vc();
                    t_flit->increment_hops(); // for stats
                    int outport;
                    if ((t_flit->get_type() == HEAD_) ||
                        (t_flit->get_type() == HEAD_TAIL_)) {

                        assert(virtualChannels[vc].get_state() == IDLE_);
                        set_vc_active(vc, curTick());
                        outport = m_router->route_compute(t_flit->get_route(0),
                            m_id, m_direction);
                        grant_outport(vc, outport);
                    } else {
                        assert(virtualChannels[vc].get_state() == ACTIVE_);
                    }
                    virtualChannels[vc].insertFlit(t_flit);
                    int vnet = vc/m_vc_per_vnet;
                    m_num_buffer_writes[vnet]++;
                    m_num_buffer_reads[vnet]++;
                    Cycles pipe_stages = m_router->get_pipe_stages();

                    if (pipe_stages == 1) {
                        t_flit->advance_stage(SA_, curTick());
                    } else {
                        assert(pipe_stages > 1);
                        Cycles wait_time = pipe_stages - Cycles(1);
                        t_flit->advance_stage(SA_, m_router->clockEdge(wait_time));

                        counter[routerID] += wait_time;
                        m_router->schedule_wakeup(Cycles(wait_time));
                    }
                }
            }

            if (roundChange)
            {
                counter[routerID] = -1;
                round_flit = 0;
                if (round_cnt < CORE_NUM-1)
                {
                    round_cnt ++;
                }
                else
                {
                    round_cnt = 0;
                    //re-generate the piority
                    std::vector<int> numbers(CORE_NUM); 
                    std::iota(numbers.begin(), numbers.end(), 0); 
                    std::random_device rd;  
                    std::mt19937 g(rd()); 
                    std::shuffle(numbers.begin(), numbers.end(), g); 
                    for (size_t i = 0; i < CORE_NUM; i++)
                    {
                        round_list[i] = numbers[i];
                    }
                }
                int nextID = round_list[round_cnt];
                counter[nextID] = 0;

                /*
                std::cout<<"  Now Change from "<<routerID<<" now counter "<<counter[routerID]<<std::endl;
                for (size_t i = 0; i < CORE_NUM; i++)
                {
                    std::cout<<" counter "<<i<<" "<<counter[i]<<std::endl;;
                }
                */
                m_router->schedule_wakeup(Cycles(THRES_SLOT));
            }
            else
            {
                counter[routerID] ++;
                m_router->schedule_wakeup(Cycles(1));
            }
        }
        // else if(lastOneStart)
        // {
        //     m_router->schedule_wakeup(Cycles(1));
        // }
        else
        {
            m_router->schedule_wakeup(Cycles(THRES_SLOT));
        }
    }
    else
    {
        flit *t_flit;
        if (m_in_link->isReady(curTick())) {

            t_flit = m_in_link->consumeLink();
            assert(t_flit->m_width == m_router->getBitWidth());
            int vc = t_flit->get_vc();
            t_flit->increment_hops(); // for stats

            if ((t_flit->get_type() == HEAD_) ||
                (t_flit->get_type() == HEAD_TAIL_)) {

                assert(virtualChannels[vc].get_state() == IDLE_);
                set_vc_active(vc, curTick());

                // Route computation for this vc
                int outport = m_router->route_compute(t_flit->get_route(0),
                    m_id, m_direction);

                // Update output port in VC
                // All flits in this packet will use this output port
                // The output port field in the flit is updated after it wins SA
                grant_outport(vc, outport);

            } else {
                assert(virtualChannels[vc].get_state() == ACTIVE_);
            }
            // Buffer the flit
            virtualChannels[vc].insertFlit(t_flit);

            int vnet = vc/m_vc_per_vnet;
            // number of writes same as reads
            // any flit that is written will be read only once
            m_num_buffer_writes[vnet]++;
            m_num_buffer_reads[vnet]++;

            Cycles pipe_stages = m_router->get_pipe_stages();
            if (pipe_stages == 1) {
                // 1-cycle router
                // Flit goes for SA directly
                t_flit->advance_stage(SA_, curTick());
            } else {
                assert(pipe_stages > 1);
                // Router delay is modeled by making flit wait in buffer for
                // (pipe_stages cycles - 1) cycles before going for SA

                Cycles wait_time = pipe_stages - Cycles(1);
                t_flit->advance_stage(SA_, m_router->clockEdge(wait_time));

                // Wakeup the router in that cycle to perform SA
                m_router->schedule_wakeup(Cycles(wait_time));
            }

            if (m_in_link->isReady(curTick())) {
                m_router->schedule_wakeup(Cycles(1));
            }
        }

    }
}

#endif

// Send a credit back to upstream router for this VC.
// Called by SwitchAllocator when the flit in this VC wins the Switch.
void
InputUnit::increment_credit(int in_vc, bool free_signal, Tick curTime)
{
    DPRINTF(RubyNetwork, "Router[%d]: Sending a credit vc:%d free:%d to %s\n",
    m_router->get_id(), in_vc, free_signal, m_credit_link->name());
    Credit *t_credit = new Credit(in_vc, free_signal, curTime);
    creditQueue.insert(t_credit);
    m_credit_link->scheduleEventAbsolute(m_router->clockEdge(Cycles(1)));
}

bool
InputUnit::functionalRead(Packet *pkt, WriteMask &mask)
{
    bool read = false;
    for (auto& virtual_channel : virtualChannels) {
        if (virtual_channel.functionalRead(pkt, mask))
            read = true;
    }

    return read;
}

uint32_t
InputUnit::functionalWrite(Packet *pkt)
{
    uint32_t num_functional_writes = 0;
    for (auto& virtual_channel : virtualChannels) {
        num_functional_writes += virtual_channel.functionalWrite(pkt);
    }

    return num_functional_writes;
}

void
InputUnit::resetStats()
{
    for (int j = 0; j < m_num_buffer_reads.size(); j++) {
        m_num_buffer_reads[j] = 0;
        m_num_buffer_writes[j] = 0;
    }
}

} // namespace garnet
} // namespace ruby
} // namespace gem5
