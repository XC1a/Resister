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

#include "mem/ruby/network/garnet/RoutingUnit.hh"

#include <iostream>		// *Taotao Xu*
#include <string>		// *Taotao Xu*
#include <utility>		// *Taotao Xu*
#include <vector>		// *Taotao Xu*

// xrwang
#include <cmath>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include "base/random.hh"
//xrwang
#include "base/cast.hh"
#include "base/compiler.hh"
#include "debug/RubyNetwork.hh"
#include "mem/ruby/network/garnet/InputUnit.hh"
#include "mem/ruby/network/garnet/Router.hh"
#include "mem/ruby/slicc_interface/Message.hh"

// *Taotao Xu*
#define MAXN 1000
unsigned long long routerr_[MAXN] = { 0 };
std::vector<std::pair<unsigned long long, unsigned long long>> times;
// *Taotao Xu*

namespace gem5
{

namespace ruby
{

namespace garnet
{

// *Taotao Xu*
extern long long counterr_ = 0;
//static long long routerr_[MAXNN] = {0};
extern unsigned long long sum = 0;
extern unsigned long long beginTime = 0;
extern unsigned long long endTime = 0;


PortDirection
RoutingUnit::compute_dirn(int index)
{
	switch(index){
	    case 0:
    		return "North";
    	case 1:
	    	return "East";
	    case 2:
	    	return "West";
    	case 3:
    	return "South";
    }
    return "Unknown";
}

int
RoutingUnit::compute_neighbor_id(int my_id, int index, int num_cols){
    switch(index){
        case 0:
        return my_id + num_cols;
        case 1:
        return my_id+1;
        case 2:
        return my_id-1;
        case 3:
        return my_id - num_cols;
    }
    return my_id;
}
// *Taotao Xu*

RoutingUnit::RoutingUnit(Router *router)
{
    m_router = router;
    m_routing_table.clear();
    m_weight_table.clear();

    //xrwang
    for (size_t i = 0; i < patternLen; i++)
    {
        patternTable.push_back({0,0,0,"Unknown",0});
    }
    //xrwang
}

void
RoutingUnit::addRoute(std::vector<NetDest>& routing_table_entry)
{
    if (routing_table_entry.size() > m_routing_table.size()) {
        m_routing_table.resize(routing_table_entry.size());
    }
    for (int v = 0; v < routing_table_entry.size(); v++) {
        m_routing_table[v].push_back(routing_table_entry[v]);
    }
}

void
RoutingUnit::addWeight(int link_weight)
{
    m_weight_table.push_back(link_weight);
}

bool
RoutingUnit::supportsVnet(int vnet, std::vector<int> sVnets)
{
    // If all vnets are supported, return true
    if (sVnets.size() == 0) {
        return true;
    }

    // Find the vnet in the vector, return true
    if (std::find(sVnets.begin(), sVnets.end(), vnet) != sVnets.end()) {
        return true;
    }

    // Not supported vnet
    return false;
}

/*
 * This is the default routing algorithm in garnet.
 * The routing table is populated during topology creation.
 * Routes can be biased via weight assignments in the topology file.
 * Correct weight assignments are critical to provide deadlock avoidance.
 */
int
RoutingUnit::lookupRoutingTable(int vnet, NetDest msg_destination)
{
    // First find all possible output link candidates
    // For ordered vnet, just choose the first
    // (to make sure different packets don't choose different routes)
    // For unordered vnet, randomly choose any of the links
    // To have a strict ordering between links, they should be given
    // different weights in the topology file

    int output_link = -1;
    int min_weight = INFINITE_;
    std::vector<int> output_link_candidates;
    int num_candidates = 0;

    // Identify the minimum weight among the candidate output links
    for (int link = 0; link < m_routing_table[vnet].size(); link++) {
        if (msg_destination.intersectionIsNotEmpty(
            m_routing_table[vnet][link])) {

        if (m_weight_table[link] <= min_weight)
            min_weight = m_weight_table[link];
        }
    }

    // Collect all candidate output links with this minimum weight
    for (int link = 0; link < m_routing_table[vnet].size(); link++) {
        if (msg_destination.intersectionIsNotEmpty(
            m_routing_table[vnet][link])) {

            if (m_weight_table[link] == min_weight) {
                num_candidates++;
                output_link_candidates.push_back(link);
            }
        }
    }

    if (output_link_candidates.size() == 0) {
        fatal("Fatal Error:: No Route exists from this Router.");
        exit(0);
    }

    // Randomly select any candidate output link
    int candidate = 0;
    if (!(m_router->get_net_ptr())->isVNetOrdered(vnet))
        candidate = rand() % num_candidates;

    output_link = output_link_candidates.at(candidate);
    return output_link;
}


void
RoutingUnit::addInDirection(PortDirection inport_dirn, int inport_idx)
{
    m_inports_dirn2idx[inport_dirn] = inport_idx;
    m_inports_idx2dirn[inport_idx]  = inport_dirn;
}

void
RoutingUnit::addOutDirection(PortDirection outport_dirn, int outport_idx)
{
    m_outports_dirn2idx[outport_dirn] = outport_idx;
    m_outports_idx2dirn[outport_idx]  = outport_dirn;
}


// only for security analysis!
    //////////////////////// 10-31
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



// outportCompute() is called by the InputUnit
// It calls the routing table by default.
// A template for adaptive topology-specific routing algorithm
// implementations using port directions rather than a static routing
// table is provided here.

int
RoutingUnit::outportCompute(RouteInfo& route, int inport,
                            PortDirection inport_dirn)
{
    int outport = -1;

    if (route.dest_router == m_router->get_id()) {

        // Multiple NIs may be connected to this router,
        // all with output port direction = "Local"
        // Get exact outport id from table

        //xrwang
        // if (SecFlag)
        //     std::cout<<"xrwang stdout ==>> GetDestination: "<<m_router->get_id()<<" curCycle: "<<m_router->curCycle()/2<<" route.sender/sink: "<<route.src_router<<" / "<<route.dest_router<<std::endl;
        //wxrwang
        outport = lookupRoutingTable(route.vnet, route.net_dest);
        return outport;
    }

    // Routing Algorithm set in GarnetNetwork.py
    // Can be over-ridden from command line using --routing-algorithm = 1
    RoutingAlgorithm routing_algorithm =
        (RoutingAlgorithm) m_router->get_net_ptr()->getRoutingAlgorithm();

    // *Taotao Xu*
    bool flag = true;
   	int num_cols = m_router->get_net_ptr()->getNumCols();
    int M5_VAR_USED num_rows = m_router->get_net_ptr()->getNumRows();

    int m_id = m_router->get_id();

    // record the pair<beginTime, endTime> of the packets transmission!!!
    // TODO
	//if (m_id == route.src_router) {
	//	beginTime = m_router->curCycle();
	//}

    switch (routing_algorithm) {
        case TABLE_:  outport =
            lookupRoutingTable(route.vnet, route.net_dest); break;	// Algorithm 0
        case XY_:     outport =
            outportComputeXY(route, inport, inport_dirn); break;	// Algorithm 1
        // any custom algorithm

		case RANWESTFIRST_ : outport =
            outportRanWestFirst(route, inport, inport_dirn); break;	// *Taotao Xu*	Algorithm 2
		case SECTEST_ : outport =
            outportSecTest(route, inport, inport_dirn); break;	// *Taotao Xu*	Algorithm 3
		case TEST_ : outport =
            outportTest(route, inport, inport_dirn); break;	// *Taotao Xu*	Algorithm 4
		case DEFENSE0_: outport =
            outportPattern(route, inport, inport_dirn); break;	// *xrwang *	Algorithm 5

        //////////////////////////// 8-20 security-valiant
		case DEFENSE1_: outport =
            outportValiant(route, inport, inport_dirn); break;	// *xrwang *	Algorithm 6

		case ORIVA: outport =
            outportOriValiant(route, inport, inport_dirn); break;	// *xrwang *	Algorithm 7

		case CUSTOM_: outport =
            outportComputeCustom(route, inport, inport_dirn); break;
        default: outport =
            lookupRoutingTable(route.vnet, route.net_dest); break;
    }

	// *Taotao Xu*
    ++counterr_;
    if (SecFlag)
    {
        printf("counter now is %lld\n", counterr_);
        printf("current time is: %lld\n", m_router->curCycle());
    }

    //xrwang
    if(SecFlag)
        std::cout<<"xrwang stdout ==>> outputport: "<<outport<<" algorithm num: "<<routing_algorithm<<" route.sender/sink: "<<route.src_router<<" / "<<route.dest_router<<std::endl;
    //wxrwang
    // if (route.dest_router == m_id) {    // *Taotao Xu*
    //     ++(routerr_[m_id]);
    // }

    ++(routerr_[m_id]);

    // printf("%d %d\n", num_cols, num_rows);   // test for debug

    if (SecFlag)
    {
        sum = 0;
        for (size_t i = 0; i < num_cols * num_rows; i++) {
            printf("router[%d] = %llu\t", i, routerr_[i]);
            sum += routerr_[i];
            if ((i + 1) % 4 == 0) printf("\n");
        }
        if (sum == counterr_) printf("The counterr_(%llu) is same with the sum(%llu) of routerr_[0] to rou									terr_[%d]\n", counterr_, sum, num_cols * num_rows - 1);

        printf("\n\n");
    }
    // *Taotao Xu*


    assert(outport != -1);

    return outport;
}

// XY routing implemented using port directions
// Only for reference purpose in a Mesh
// By default Garnet uses the routing table
int
RoutingUnit::outportComputeXY(RouteInfo route,
                              int inport,
                              PortDirection inport_dirn)
{
    PortDirection outport_dirn = "Unknown";

    [[maybe_unused]] int num_rows = m_router->get_net_ptr()->getNumRows();
    int num_cols = m_router->get_net_ptr()->getNumCols();
    assert(num_rows > 0 && num_cols > 0);

    int my_id = m_router->get_id();
    int my_x = my_id % num_cols;
    int my_y = my_id / num_cols;

    int dest_id = route.dest_router;
    int dest_x = dest_id % num_cols;
    int dest_y = dest_id / num_cols;

    int x_hops = abs(dest_x - my_x);
    int y_hops = abs(dest_y - my_y);

    bool x_dirn = (dest_x >= my_x);
    bool y_dirn = (dest_y >= my_y);

    // already checked that in outportCompute() function
    assert(!(x_hops == 0 && y_hops == 0));

    if (x_hops > 0) {
        if (x_dirn) {
            assert(inport_dirn == "Local" || inport_dirn == "West");
            outport_dirn = "East";
        } else {
            assert(inport_dirn == "Local" || inport_dirn == "East");
            outport_dirn = "West";
        }
    } else if (y_hops > 0) {
        if (y_dirn) {
            // "Local" or "South" or "West" or "East"
            assert(inport_dirn != "North");
            outport_dirn = "North";
        } else {
            // "Local" or "North" or "West" or "East"
            assert(inport_dirn != "South");
            outport_dirn = "South";
        }
    } else {
        // x_hops == 0 and y_hops == 0
        // this is not possible
        // already checked that in outportCompute() function
        panic("x_hops == y_hops == 0");
    }

    return m_outports_dirn2idx[outport_dirn];
}

int
RoutingUnit::outportComputeXY(RouteInfo route,
                              int inport,
                              PortDirection inport_dirn, bool mid)
{
    PortDirection outport_dirn = "Unknown";

    [[maybe_unused]] int num_rows = m_router->get_net_ptr()->getNumRows();
    int num_cols = m_router->get_net_ptr()->getNumCols();
    assert(num_rows > 0 && num_cols > 0);

    int my_id = m_router->get_id();
    int my_x = my_id % num_cols;
    int my_y = my_id / num_cols;

    int dest_id;
    if (mid)
        dest_id = route.getInter();
    else
        dest_id = route.dest_router;

    int dest_x = dest_id % num_cols;
    int dest_y = dest_id / num_cols;

    int x_hops = abs(dest_x - my_x);
    int y_hops = abs(dest_y - my_y);

    bool x_dirn = (dest_x >= my_x);
    bool y_dirn = (dest_y >= my_y);

    // already checked that in outportCompute() function
    assert(!(x_hops == 0 && y_hops == 0));

    if (x_hops > 0) {
        if (x_dirn) {
            // assert(inport_dirn == "Local" || inport_dirn == "West");
            outport_dirn = "East";
        } else {
            // assert(inport_dirn == "Local" || inport_dirn == "East");
            outport_dirn = "West";
        }
    } else if (y_hops > 0) {
        if (y_dirn) {
            // "Local" or "South" or "West" or "East"
            // assert(inport_dirn != "North");
            outport_dirn = "North";
        } else {
            // "Local" or "North" or "West" or "East"
            // assert(inport_dirn != "South");
            outport_dirn = "South";
        }
    } else {
        // x_hops == 0 and y_hops == 0
        // this is not possible
        // already checked that in outportCompute() function
        panic("x_hops == y_hops == 0");
    }

    return m_outports_dirn2idx[outport_dirn];
}


int
RoutingUnit::outportValiant(RouteInfo& route,
                              int inport,
                              PortDirection inport_dirn)
{
    if(SecFlag)
        std::cout<<"  Now is secure-valiant algorithm: params "<<route.getTag()<<" "<<route.getInter()<<" "<<route.getPassed()<<" nowRouter "<<m_router->get_id()<<" src and dst "<<route.src_router<<" "<<route.dest_router<<std::endl;

    int res;
    if (route.getTag() == 0)
    {
        res = outportComputeXY(route, inport, inport_dirn, false);
    }
    else
    {
        if ((route.getInter() != -1) && (route.getTag() == 1) && (!route.getPassed()))
        {
            if (route.getInter() == m_router->get_id())
            {
                if(SecFlag)
                    std::cout<<"** secure-valiant algorithm: passed inter"<<std::endl;
                route.setPassed();
            }
        }

        if (route.getInter() == -1) // first allocate a new inter node
        {
            [[maybe_unused]] int num_rows = m_router->get_net_ptr()->getNumRows();
            int num_cols = m_router->get_net_ptr()->getNumCols();
            assert(num_rows > 0 && num_cols > 0);
            int my_id = m_router->get_id();
            int my_x = my_id % num_cols;
            int my_y = my_id / num_cols;
            int dest_id;
            dest_id = route.dest_router;
            int dest_x = dest_id % num_cols;
            int dest_y = dest_id / num_cols;
            int x_hops = abs(dest_x - my_x);
            int y_hops = abs(dest_y - my_y);
            bool x_dirn = (dest_x >= my_x);
            bool y_dirn = (dest_y >= my_y);
            if ((x_hops>0) && (y_hops == 0) )
            {
                //choose from source node
                if(SecFlag)
                    std::cout<<"** secure-valiant algorithm setting inter: x_dir"<<std::endl;
                std::vector<int> my_y_option;
                for (size_t i = 0; i < num_cols; i++)
                {
                    if (i != my_y)
                    {
                        my_y_option.push_back(i);
                    }
                }
                // int rNum = random_mt.random<int>(0, (int) (num_cols - 2)); // !!!!!! -2
                int rNum = mR.getMyRandom(0, (int) (num_cols - 2)); // !!!!!! -2
                int newY = my_y_option[rNum];
                if(SecFlag)
                    std::cout<<"** set new Inter "<<newY<<" "<<num_cols<<" "<<my_x<<" "<<rNum<<std::endl;
                route.setInter(newY*num_cols + my_x);
            }
            else if ((y_hops>0) && (x_hops == 0) )
            {
                //choose from dst node
                assert(y_hops>0);
                if(SecFlag)
                    std::cout<<"** secure-valiant algorithm setting inter: y_dir destInfo "<<dest_x<<" "<<dest_y<<std::endl;

                std::vector<int> dest_x_option;
                for (size_t i = 0; i < num_cols; i++)
                {
                    if (i != dest_x)
                    {
                        dest_x_option.push_back(i);
                    }
                }
                // int rNum = random_mt.random<int>(0, (int) (num_cols - 2));
                int rNum = mR.getMyRandom(0, (int) (num_cols - 2));
                int newX = dest_x_option[rNum];
                route.setInter(dest_y*num_cols + newX);
            }
            else if ((y_hops>0) && (x_hops > 0) )
            {
                if(SecFlag)
                    std::cout<<"** secure-valiant algorithm setting inter: xy_dir"<<std::endl;
                std::vector<int> dest_x_option;
                for (size_t i = 0; i < num_cols; i++)
                {
                    if (i != dest_x)
                    {
                        dest_x_option.push_back(i);
                    }
                }
                // int rNum = random_mt.random<int>(0, (int) (num_cols - 2));
                int rNum = mR.getMyRandom(0, (int) (num_cols - 2));
                int newX = dest_x_option[rNum];
                route.setInter(newX + dest_y*num_cols);
            }
            if(SecFlag)
                std::cout<<"** secure-valiant algorithm: set newInter "<<route.getInter()<<std::endl;

            res = outportComputeXY(route, inport, inport_dirn, true);
        }
        else
        {
            if(SecFlag)
                std::cout<<"** secure-valiant algorithm: working"<<std::endl;
            if (!route.getPassed())
            {
                res = outportComputeXY(route, inport, inport_dirn, true);
            }
            else
            {
                res = outportComputeXY(route, inport, inport_dirn, false);
            }
        }
    }

    return res;
}

int
RoutingUnit::outportOriValiant(RouteInfo& route,
                              int inport,
                              PortDirection inport_dirn)
{
    if(SecFlag)
        std::cout<<"  Now is secure-valiant algorithm: params "<<route.getTag()<<" "<<route.getInter()<<" "<<route.getPassed()<<" nowRouter "<<m_router->get_id()<<" src and dst "<<route.src_router<<" "<<route.dest_router<<std::endl;

    int res;
    if (route.getTag() == 0)
    {
        res = outportComputeXY(route, inport, inport_dirn, false);
    }
    else
    {
        if ((route.getInter() != -1) && (route.getTag() == 1) && (!route.getPassed()))
        {
            if (route.getInter() == m_router->get_id())
            {
                if(SecFlag)
                    std::cout<<"** secure-valiant algorithm: passed inter"<<std::endl;
                route.setPassed();
            }
        }

        if (route.getInter() == -1) // first allocate a new inter node
        {
            [[maybe_unused]] int num_rows = m_router->get_net_ptr()->getNumRows();
            int num_cols = m_router->get_net_ptr()->getNumCols();
            assert(num_rows > 0 && num_cols > 0);
            int my_id = m_router->get_id();
            int my_x = my_id % num_cols;
            int my_y = my_id / num_cols;
            int dest_id;
            dest_id = route.dest_router;
            int dest_x = dest_id % num_cols;
            int dest_y = dest_id / num_cols;
            int x_hops = abs(dest_x - my_x);
            int y_hops = abs(dest_y - my_y);
            bool x_dirn = (dest_x >= my_x);
            bool y_dirn = (dest_y >= my_y);

            //choose from source node
            std::vector<int> my_option;
            for (size_t i = 0; i < (num_cols*num_rows); i++)
            {
                if ((i != my_id) && (i != dest_id))
                {
                    my_option.push_back(i);
                }
            }

            // int rNum = random_mt.random<int>(0, (int) ((num_cols*num_rows)- 3)); // !!!!!! -2
            int rNum = mR.getMyRandom(0, (int) ((num_cols*num_rows)- 3));
            int newNode = my_option[rNum];
            if(SecFlag)
                std::cout<<"** set new Inter "<<newNode<<" "<<rNum<<std::endl;
            route.setInter(newNode);
        
            if(SecFlag)
                std::cout<<"** secure-valiant algorithm: set newInter "<<route.getInter()<<std::endl;

            res = outportComputeXY(route, inport, inport_dirn, true);
        }
        else
        {
            if(SecFlag)
                std::cout<<"** secure-valiant algorithm: working"<<std::endl;
            if (!route.getPassed())
            {
                res = outportComputeXY(route, inport, inport_dirn, true);
            }
            else
            {
                res = outportComputeXY(route, inport, inport_dirn, false);
            }
        }
    }

    return res;
}



////////////////////// xrwang
int
RoutingUnit::outportPattern(RouteInfo route,
                              int inport,
                              PortDirection inport_dirn)
{
    PortDirection outport_dirn = "Unknown";

    [[maybe_unused]] int num_rows = m_router->get_net_ptr()->getNumRows();
    int num_cols = m_router->get_net_ptr()->getNumCols();
    assert(num_rows > 0 && num_cols > 0);

    int my_id = m_router->get_id();
    int my_x = my_id % num_cols;
    int my_y = my_id / num_cols;

    int dest_id = route.dest_router;
    int dest_x = dest_id % num_cols;
    int dest_y = dest_id / num_cols;

    int x_hops = abs(dest_x - my_x);
    int y_hops = abs(dest_y - my_y);

    bool x_dirn = (dest_x >= my_x);
    bool y_dirn = (dest_y >= my_y);

    // already checked that in outportCompute() function
    assert(!(x_hops == 0 && y_hops == 0));
    //to track risks
    patternEntry riskEntry{0,0,0,"Unknown",0};
    bool hasRisk = riskFind(riskEntry);
    if (hasRisk)
    {
        bool samePkt = riskEntry.dst == dest_id && riskEntry.src == route.src_router;
        hasRisk = hasRisk && samePkt;
    }
    if (!hasRisk)
    {
        //we randomly choose west or east
        double injRange = pow((double) 10, (double) 2);
        unsigned trySending = random_mt.random<unsigned>(0, (int) injRange);
        if (trySending < injRange*0.5)
        {
            if (x_hops > 0) {
                if (x_dirn) {
                    outport_dirn = "East";
                } else {
                    outport_dirn = "West";
                }
            } else if (y_hops > 0) {
                if (y_dirn) {
                    outport_dirn = "North";
                } else {
                    outport_dirn = "South";
                }
            } else {
                panic("x_hops == y_hops == 0");
            }
        }
        else
        {
            if (y_hops > 0) {
                if (y_dirn) {
                    outport_dirn = "North";
                } else {
                    outport_dirn = "South";
                }
            }
            else if (x_hops > 0) {
                if (x_dirn) {
                    outport_dirn = "East";
                } else {
                    outport_dirn = "West";
                }
            }
            else {
                panic("x_hops == y_hops == 0");
            }
        }
    }
    else
    {
        if(SecFlag)
            std::cout<<"xrwang stdout ==>> risk detected!!! "<<std::endl;
        PortDirection former_outport_dirn = riskEntry.output;
        // PortDirection former_inport_dirn = "Unknown";
        std::vector<PortDirection> allList = {"East","West","South","North"};
        allList.erase(remove(allList.begin(), allList.end(), former_outport_dirn), allList.end());
        //we should knick out the oppsite outport
        PortDirection delete_dir = "Unknown";
        if (y_dirn)
        {
            delete_dir = "South";
        }
        else if (!y_dirn)
        {
            delete_dir = "North";
        }
        else if (x_dirn)
        {
            delete_dir = "West";
        }
        else if (!x_dirn)
        {
            delete_dir = "East";
        }
        allList.erase(remove(allList.begin(), allList.end(), delete_dir), allList.end());


        PortDirection now_outport_dirn = "Unknown";
        unsigned rNum = random_mt.random<unsigned>(0, (int) 100);
        int chooseId = 0;
        if (rNum < 50)
        {
            chooseId = 0;
            now_outport_dirn = allList[0];
        }
        else
        {
            chooseId = 1;
            now_outport_dirn = allList[1];
        }

        bool boundConflic = (now_outport_dirn == "South" && my_y == 0) || (now_outport_dirn == "North" && my_y == num_rows-1) ||
                                (now_outport_dirn == "West" && my_x == 0) || (now_outport_dirn == "East" && my_x == num_cols-1);
        if (boundConflic)
        {
            now_outport_dirn = chooseId == 0 ? allList[1] : allList[0];
        }

        outport_dirn = now_outport_dirn;
        if(SecFlag)
            std::cout<<"xrwang stdout ==>> redirect to: "<<now_outport_dirn<<std::endl;
    }

    //update the pattern table
    patternTable.erase(patternTable.begin());
    patternTable.push_back({route.src_router,route.dest_router,inport,outport_dirn,curTick()/500});
    if(SecFlag)
    {
        std::cout<<"ID is "<<m_router->get_id()<<" out direction: "<<outport_dirn<<std::endl;
        printTable();
    }

    return m_outports_dirn2idx[outport_dirn];
}

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
    return variance / size;
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
RoutingUnit::entryCmp(patternEntry& AEntry, patternEntry& BEntry)
{
    return AEntry.src == BEntry.src && AEntry.dst == BEntry.dst && AEntry.output == BEntry.output;
}


bool
RoutingUnit::riskFind(patternEntry& riskEntry)
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
    if (!find)
        return false;

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
    double variance = ComputeVariance(timePatternDiff.begin(),timePatternDiff.end());
    double avg= ComputeAvg(timePattern);
    if(SecFlag)
        std::cout<<"The variance: "<<variance<<" avg: "<<avg<<std::endl;

    // if (variance <= 1 && (std::abs(avg) < 6))
    if (variance <= 3 && (std::abs(avg) < 6)) ///5.8
        return true;
    else
        return false;
}



///////////////////////////////// xrwang

// *Taotao Xu*
int
RoutingUnit::outportRanWestFirst(RouteInfo route,
                                 int inport,
                                 PortDirection inport_dirn)
{
    //assert(0);
    PortDirection outport_dirn = "Unknown";

    int avail_dimension_set[] = {0,0,0,0};
                            // N, E, W, S

    int M5_VAR_USED num_rows = m_router->get_net_ptr()->getNumRows();
    int num_cols = m_router->get_net_ptr()->getNumCols();
    assert(num_rows > 0 && num_cols > 0);

    int my_id = m_router->get_id();
    int cx = my_id % num_cols;
    int cy = my_id / num_cols;

    int dest_id = route.dest_router;
    int dx = dest_id % num_cols;
    int dy = dest_id / num_cols;

    int source_id = route.src_router;

    int ex = (dx - cx);
    int ey = (dy - cy);

    // already checked that in outportCompute() function
    assert(!(ex == 0 && ey == 0));

    //  West-First algo
    if(ex < 0)  //When we need to go West
    {
        avail_dimension_set[2]++;
    }
    else{       // No restrictions otherwise
        if(ex > 0) avail_dimension_set[1]++;

        if(ey > 0) avail_dimension_set[0]++;
        else if(ey < 0) avail_dimension_set[3]++;
    }
    // End of West First algorithm

    int dir_count = 0;
    printf("S %d\tD %d\tC %d\t going to ", source_id, dest_id, my_id);
    for(int i = 0; i < 4; i++) {
        dir_count += avail_dimension_set[i];

        if(avail_dimension_set[i]) {     //handles dir_count = 1 case
            outport_dirn = this->compute_dirn(i);
        }
        if(avail_dimension_set[i] > 0) {
            //printf("S %d D %d Router: %d\t going to %s\n",source_id,dest_id,my_id,(this->compute_dir						n(i)).c_str());
            printf("%s ",(this->compute_dirn(i)).c_str());
        }
    }

    if (dir_count == 2) {
        int i1 = -1, i2 = -1;
        for(int i = 0; i < 4; i++) {
            if(avail_dimension_set[i] > 0) {
                if (i1 == -1) i1 = i;
                else i2 = i;
            }
        }
        std::srand(std::time(nullptr));
        int random_var = std::rand();
        if(random_var % 2 == 0){
            outport_dirn = this->compute_dirn(i1);
        }
        else{
            outport_dirn = this->compute_dirn(i2);
        }
        printf("-- %s",outport_dirn.c_str());
    }
    printf("\n");

    return m_outports_dirn2idx[outport_dirn];
}
// *Taotao Xu*


//  *Taotao Xu*
//  Algorithm 3 Security for counter function test
int
RoutingUnit::outportSecTest(RouteInfo route,
                             int inport,
                             PortDirection inport_dirn)
{
	//assert(0);
    PortDirection outport_dirn = "Unknown";

    int avail_dimension_set[] = {0,0,0,0};
                            // N, E, W, S

    int M5_VAR_USED num_rows = m_router->get_net_ptr()->getNumRows();
    int num_cols = m_router->get_net_ptr()->getNumCols();
    assert(num_rows > 0 && num_cols > 0);

    int my_id = m_router->get_id();
    int cx = my_id % num_cols;
    int cy = my_id / num_cols;

    int dest_id = route.dest_router;
    int dx = dest_id % num_cols;
    int dy = dest_id / num_cols;

    int source_id = route.src_router;

    int ex = (dx - cx);
    int ey = (dy - cy);

    // already checked that in outportCompute() function
    assert(!(ex == 0 && ey == 0));

    //  West-First algo
    if(ex < 0)  //When we need to go West
    {
        avail_dimension_set[2]++;
    }
    else{       // No restrictions otherwise
        if(ex > 0) avail_dimension_set[1]++;

        if(ey > 0) avail_dimension_set[0]++;
        else if(ey < 0) avail_dimension_set[3]++;
    }

   	int dir_count = 0;
    printf("S %d\tD %d\tC %d\t going to ", source_id, dest_id, my_id);
    for(int i = 0; i < 4; i++) {
        dir_count += avail_dimension_set[i];

        if (avail_dimension_set[i]) {     // handles dir_count = 1 case
            outport_dirn = this->compute_dirn(i);
        }
        if (avail_dimension_set[i] > 0) {
            //printf("S %d D %d Router: %d\t going to %s\n", source_id, dest_id, my_id, (this->compute_dir						n(i)).c_str());
            printf("%s ", (this->compute_dirn(i)).c_str());
        }
    }

	if (dir_count == 2) {
		if (cy != 0 || cy != num_cols || cx != 0 || cx != num_cols) {
			int cnt1 = this->compute_neighbor_id(my_id, 0, num_cols);
			int cnt2 = this->compute_neighbor_id(my_id, 1, num_cols);
			int cnt3 = this->compute_neighbor_id(my_id, 2, num_cols);
            int cnt4 = this->compute_neighbor_id(my_id, 3, num_cols);

            std::vector<unsigned long long> tmp = { routerr_[cnt1], routerr_[cnt2], routerr_[cnt3], routerr_[cnt4]};
            for (auto &i : tmp)
				std::cout << i << " ";
            std::cout << "\n";

			unsigned long long res = *min_element(tmp.begin(), tmp.end());

			if (res == routerr_[cnt1]) outport_dirn = this->compute_dirn(0);
			else if (res == routerr_[cnt2]) outport_dirn = this->compute_dirn(1);
			else if(res == routerr_[cnt3]) outport_dirn = this->compute_dirn(2);
            else if (res == routerr_[cnt4]) outport_dirn = this->compute_dirn(3);

			printf("-- %s", outport_dirn.c_str());
		}
		int i1 = -1, i2 = -1;
        for(int i = 0; i < 4; i++) {
            if(avail_dimension_set[i] > 0){
                if (i1 == -1) i1 = i;
                else i2 = i;
            }
        }
        std::srand(std::time(nullptr));
        int random_var = std::rand();
        if(random_var % 2 == 0){
            outport_dirn = this->compute_dirn(i1);
        }
        else{
            outport_dirn = this->compute_dirn(i2);
        }
        printf("-- %s",outport_dirn.c_str());
	}
	printf("\n");
	return m_outports_dirn2idx[outport_dirn];
}


// *Taotao Xu*
// Algorithm 4 random routing by means of counter function!
// Adaptive Routing Algorithms with some limitations!!!
int
RoutingUnit::outportTest(RouteInfo route,
						int inport,
                        PortDirection inport_dirn)
{
   	// panic("%s placeholder executed", __FUNCTION__);
    //assert(0);

    unsigned long long route_cost = 0;	// calculate the route cost for each src/dest pair nodes.
	PortDirection outport_dirn = "Unknown";
	int M5_VAR_USED num_rows = m_router->get_net_ptr()->getNumRows();
	int num_cols = m_router->get_net_ptr()->getNumCols();
	assert(num_rows > 0 && num_cols > 0);

	int my_id = m_router->get_id();
	int dest_id = route.dest_router;
	int source_id = route.src_router;

	int sx = source_id % num_cols;
	int sy = source_id / num_cols;

	int dx = dest_id % num_cols;
    int dy = dest_id / num_cols;

    route_cost = abs(sx - dx) + abs(sy - dy);

	int cnt1 = this->compute_neighbor_id(my_id, 0, num_cols);
	int cnt2 = this->compute_neighbor_id(my_id, 1, num_cols);
	int cnt3 = this->compute_neighbor_id(my_id, 2, num_cols);
    int cnt4 = this->compute_neighbor_id(my_id, 3, num_cols);


	// conditions for choose the next router by means of the counter values!!!
	if (cnt1 >= num_cols * num_rows) { cnt1 = 900; routerr_[cnt1] = 0x7fffffff; }
	if (cnt2 >= num_cols * num_rows || cnt2 / num_cols != my_id / num_cols) { cnt2 = 901; routerr_[cnt2] = 0x7fffffff; }
	if (cnt3 < 0 || cnt3 / num_cols != my_id / num_cols) { cnt3 = 902; routerr_[cnt3] = 0x7fffffff; }
	if (cnt4 < 0) { cnt4 = 903; routerr_[cnt4] = 0x7fffffff; }


    std::vector<unsigned long long> tmp = { routerr_[cnt1], routerr_[cnt2], routerr_[cnt3], routerr_[cnt4]};

    // just for test.
    for (auto &i: tmp)
		std::cout << i << " ";
	std::cout << std::endl;

    printf("S %d\tD %d\tC %d\t going to ", source_id, dest_id, my_id);

	unsigned long long res = *min_element(tmp.begin(), tmp.end());

    // printf("%llu\n", res); // for test.
	if (res >= 0) {			// eliminate bug segmentation fault
		if (res == routerr_[cnt1]) outport_dirn = this->compute_dirn(0);
		else if (res == routerr_[cnt2]) outport_dirn = this->compute_dirn(1);
		else if (res == routerr_[cnt3]) outport_dirn = this->compute_dirn(2);
        else if (res == routerr_[cnt4]) outport_dirn = this->compute_dirn(3);
   	}

	printf("-- %s", outport_dirn.c_str());
    printf("\n");

	printf("The shortest path cost is %llu\n", route_cost);
	return m_outports_dirn2idx[outport_dirn];
}

// Template for implementing custom routing algorithm
// using port directions. (Example adaptive)
int
RoutingUnit::outportComputeCustom(RouteInfo route,
                                 int inport,
                                 PortDirection inport_dirn)
{
    panic("%s placeholder executed", __FUNCTION__);
}

} // namespace garnet
} // namespace ruby
} // namespace gem5
