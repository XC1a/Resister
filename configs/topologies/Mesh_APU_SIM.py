# Copyright (c) 2010 Advanced Micro Devices, Inc.
#               2016 Georgia Institute of Technology
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# by xrwang 2024.6.4

from __future__ import print_function
from __future__ import absolute_import

from m5.params import *
from m5.objects import *

from common import FileSystemConfig

from topologies.BaseTopology import SimpleTopology

# Creates a generic Mesh assuming an equal number of cache
# and directory controllers.
# XY routing is enforced (using link weights)
# to guarantee deadlock freedom.

def divisible(a, b):
    return (int(a/b) * b == a)

CPU_CORE=4 # APU fixed 4 cores

class Mesh_APU_SIM(SimpleTopology):
    description='NoI mesh baseline'

    def __init__(self, controllers):
        self.nodes = controllers

    # Makes a generic mesh
    # assuming an equal number of cache and directory cntrls

    def makeTopology(self, options, network, IntLink, ExtLink, Router):
        nodes = self.nodes

        num_noc_routers = options.num_cpus 
        num_GPU_routers = options.num_cpus - options.num_apu_cores# xrwang: total routers
        num_cpu=options.num_apu_cores
        num_interposer_routers = options.num_chiplets * options.num_boundary_routers # xrwang: total routers
        num_rows = options.mesh_rows
        num_chiplets = options.num_chiplets # describe how many GPU cores: typical is 16 CUs per GPU core
        cpus_per_chiplet, remainder = divmod(num_GPU_routers,
                        options.num_chiplets)
        assert(remainder == 0)
        num_chiplet_rows = int(cpus_per_chiplet**(.5))
        num_chiplet_columns, remainder = divmod(cpus_per_chiplet,
                        num_chiplet_rows)
        while remainder != 0:
            num_chiplet_rows -= 1
            num_chiplet_columns, remainder = divmod(cpus_per_chiplet,
                            num_chiplet_rows)
        print ('CUs_per_chiplet:', cpus_per_chiplet,
                '   #chiplet_rows:', num_chiplet_rows,
                '   #chiplet_columns:', num_chiplet_columns)
        assert(num_chiplet_rows > 0)
        assert(remainder == 0)
        assert(num_chiplet_rows <= num_rows)
        assert(divisible(num_rows, num_chiplet_rows)) # True if divisible

        # default values for link latency and router latency.
        # Can be over-ridden on a per link/router basis
        link_latency = options.link_latency # used by simple and garnet
        router_latency = options.router_latency # only used by garnet

        # There must be an evenly divisible number of cntrls to routers
        # Also, obviously the number or rows must be <= the number of routers
        print('node num: ',len(nodes))
        cntrls_per_router, remainder = divmod(len(nodes), num_noc_routers)
        print("cntrl per router: ",cntrls_per_router)
        assert(num_rows > 0 and num_rows <= num_noc_routers)
        num_columns = int(num_GPU_routers / num_rows)
        assert(num_columns * num_rows == num_GPU_routers)
        #assert(remainder == 4 or num_noc_routers == 4) # by YenHao
        assert(divisible(num_columns, num_chiplet_columns)) # True if divisible
        # num_noi_routers = num_noc_routers # Mesh NoI topology
        num_noi_routers = num_chiplets * options.num_boundary_routers# xrwang: each GPU chiplets set x boundary routers
        print('#NoC_routers:', num_noc_routers,
                '   #NoI_routers:', num_noi_routers)

        # Create the NoC routers in the mesh
        noc_routers = [Router(router_id=i, latency = router_latency) \
            for i in range(num_noc_routers)]
        # Create the NoI routers on the interposer
        noi_routers = [Router(router_id=i+num_noc_routers,
                        latency = router_latency) \
            for i in range(num_noi_routers)]
        network.routers = noc_routers + noi_routers

        # link counter to set unique link ids
        link_count = 0

        # The remainder_nodes are Dir nodes as the memory controller node
        network_nodes = []
        remainder_nodes = []
        for node_index in range(len(nodes)):
            #if node_index < num_noc_routers:
            if node_index < (len(nodes) - remainder):
                network_nodes.append(nodes[node_index])
            else:
                remainder_nodes.append(nodes[node_index])

        # Connect each node to the appropriate router
        print('network nodes num: ',len(network_nodes))
        ext_l1cache_links = []
        ext_dir_links = []
        for (i, n) in enumerate(network_nodes):
            # print(i,n)
            if (n.type == 'L1Cache_Controller'):
                cntrl_level, router_id = divmod(i, num_noc_routers)
                assert(cntrl_level < cntrls_per_router)
                ext_l1cache_links.append(ExtLink(link_id=link_count,
                                        ext_node=n,
                                        int_node=noc_routers[router_id],
                                        latency = link_latency))
                link_count += 1
        
        #################
        ###########
        #### xrwang: only 8 MCs are configured, are connected with interposer routers
        ###########
        #################

        # Connect the 4 remainding nodes to corners.  These should only be
        # Dir nodes.
        # print('mem controller nodes: ',remainder_nodes)
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[0],
                                int_node=noi_routers[0],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[1],
                                int_node=noi_routers[4],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[2],
                                int_node=noi_routers[8],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[3],
                                int_node=noi_routers[12],
                                latency = link_latency))
        link_count += 1

        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[4],
                                int_node=noi_routers[3],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[5],
                                int_node=noi_routers[7],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[6],
                                int_node=noi_routers[11],
                                latency = link_latency))
        link_count += 1
        ext_dir_links.append(ExtLink(link_id=link_count,
                                ext_node=remainder_nodes[7],
                                int_node=noi_routers[15],
                                latency = link_latency))
        link_count += 1

        network.ext_links = ext_l1cache_links + ext_dir_links
        print('#external_links:', len(network.ext_links),
                '   #L1_cache_links:', len(ext_l1cache_links),
                '   #ext_dir_links:', len(ext_dir_links))

        # Create internal links between routers.
        ####################
        #### xrwang connect GPU chiplets routers
        ####################
        int_noc_links = []

        # East output to West input links (weight = 1)
        for row in range(num_rows):
            for col in range(num_columns):
                if (col + 1 < num_columns
                                and not divisible(col+1, num_chiplet_columns)):
                    east_out = col + (row * num_columns)
                    west_in = (col + 1) + (row * num_columns)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[east_out],
                                             dst_node=noc_routers[west_in],
                                             src_outport="East",
                                             dst_inport="West",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1

        # West output to East input links (weight = 1)
        for row in range(num_rows):
            for col in range(num_columns):
                if (col + 1 < num_columns
                                and not divisible(col+1, num_chiplet_columns)):
                    east_in = col + (row * num_columns)
                    west_out = (col + 1) + (row * num_columns)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[west_out],
                                             dst_node=noc_routers[east_in],
                                             src_outport="West",
                                             dst_inport="East",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1

        # North output to South input links (weight = 2)
        for col in range(num_columns):
            for row in range(num_rows):
                if (row + 1 < num_rows
                                and not divisible(row+1, num_chiplet_rows)):
                    north_out = col + (row * num_columns)
                    south_in = col + ((row + 1) * num_columns)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[north_out],
                                             dst_node=noc_routers[south_in],
                                             src_outport="North",
                                             dst_inport="South",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1

        # South output to North input links (weight = 2)
        for col in range(num_columns):
            for row in range(num_rows):
                if (row + 1 < num_rows
                                and not divisible(row+1, num_chiplet_rows)):
                    north_in = col + (row * num_columns)
                    south_out = col + ((row + 1) * num_columns)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[south_out],
                                             dst_node=noc_routers[north_in],
                                             src_outport="South",
                                             dst_inport="North",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1

        ####################
        #### xrwang connect CPU chiplets routers
        ####################
        cpu_row = int(options.num_apu_cores**0.5)
        cpu_col = int(options.num_apu_cores**0.5)
        for row in range(cpu_row):
            for col in range(cpu_col):
                if (col + 1 < cpu_col):
                    east_out = col + (row * cpu_col)
                    west_in = (col + 1) + (row * cpu_col)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[east_out+num_GPU_routers],
                                             dst_node=noc_routers[west_in+num_GPU_routers],
                                             src_outport="East",
                                             dst_inport="West",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1
        for row in range(cpu_row):
            for col in range(cpu_col):
                if (col + 1 < cpu_col):
                    east_in = col + (row * cpu_col)
                    west_out = (col + 1) + (row * cpu_col)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[west_out+num_GPU_routers],
                                             dst_node=noc_routers[east_in+num_GPU_routers],
                                             src_outport="West",
                                             dst_inport="East",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1
        for col in range(cpu_col):
            for row in range(cpu_row):
                if (row + 1 < cpu_row):
                    north_out = col + (row * cpu_col)
                    south_in = col + ((row + 1) * cpu_col)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[north_out+num_GPU_routers],
                                             dst_node=noc_routers[south_in+num_GPU_routers],
                                             src_outport="North",
                                             dst_inport="South",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1
        for col in range(cpu_col):
            for row in range(cpu_row):
                if (row + 1 < cpu_row):
                    north_in = col + (row * cpu_col)
                    south_out = col + ((row + 1) * cpu_col)
                    int_noc_links.append(IntLink(link_id=link_count,
                                             src_node=noc_routers[south_out+num_GPU_routers],
                                             dst_node=noc_routers[north_in+num_GPU_routers],
                                             src_outport="South",
                                             dst_inport="North",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1

        ####################
        #### xrwang connect interposer routers
        ####################
        # Connect network-on-interposer (NoI) internal routers
        int_noi_links = []

        # East output to West input links (weight = 1)
        interposer_row = int(num_interposer_routers**0.5)
        interposer_col = int(num_interposer_routers**0.5)
        print('interposer size: ',interposer_row,' * ',interposer_col)
        for row in range(interposer_row):
            for col in range(interposer_col):
                if (col + 1 < interposer_col):
                    east_out = col + (row * interposer_col)
                    west_in = (col + 1) + (row * interposer_col)
                    int_noi_links.append(IntLink(link_id=link_count,
                                             src_node=noi_routers[east_out],
                                             dst_node=noi_routers[west_in],
                                             src_outport="East",
                                             dst_inport="West",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1
        for row in range(interposer_row):
            for col in range(interposer_col):
                if (col + 1 < interposer_col):
                    east_in = col + (row * interposer_col)
                    west_out = (col + 1) + (row * interposer_col)
                    int_noi_links.append(IntLink(link_id=link_count,
                                             src_node=noi_routers[west_out],
                                             dst_node=noi_routers[east_in],
                                             src_outport="West",
                                             dst_inport="East",
                                             latency = link_latency,
                                             weight=1))
                    link_count += 1
        for col in range(interposer_col):
            for row in range(interposer_row):
                if (row + 1 < interposer_row):
                    north_out = col + (row * interposer_col)
                    south_in = col + ((row + 1) * interposer_col)
                    int_noi_links.append(IntLink(link_id=link_count,
                                             src_node=noi_routers[north_out],
                                             dst_node=noi_routers[south_in],
                                             src_outport="North",
                                             dst_inport="South",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1
        for col in range(interposer_col):
            for row in range(interposer_row):
                if (row + 1 < interposer_row):
                    north_in = col + (row * interposer_col)
                    south_out = col + ((row + 1) * interposer_col)
                    int_noi_links.append(IntLink(link_id=link_count,
                                             src_node=noi_routers[south_out],
                                             dst_node=noi_routers[north_in],
                                             src_outport="South",
                                             dst_inport="North",
                                             latency = link_latency,
                                             weight=2))
                    link_count += 1


        ####################
        #### xrwang connect interposer routers with noc routers
        #### boudary routers per chiplets
        ####################
        noc_map_noi_col = [1,2,5,6]
        noc_map_noi_row = [0,3,4,7]
        # Connect network-on-chip (NoC) and network-on-interposer (NoI) routers
        int_chiplet_interposer_links = []

        for col in range(interposer_col):
            for row in range(interposer_row):
                noc_in = noc_map_noi_col[col] + (noc_map_noi_row[row] * num_columns)
                noi_out = col + (row * interposer_col)
                int_chiplet_interposer_links.append(
                        IntLink(link_id=link_count,
                            src_node=noi_routers[noi_out],
                            dst_node=noc_routers[noc_in],
                            src_outport="NoI",
                            dst_inport="NoC",
                            latency = link_latency,
                            weight=3))
                link_count += 1
        for col in range(interposer_col):
            for row in range(interposer_row):
                if (True):
                    noc_out = noc_map_noi_col[col] + (noc_map_noi_row[row] * num_columns)
                    noi_in = col + (row * interposer_col)
                    int_chiplet_interposer_links.append(
                            IntLink(link_id=link_count,
                                src_node=noc_routers[noc_out],
                                dst_node=noi_routers[noi_in],
                                src_outport="NoC",
                                dst_inport="NoI",
                                latency = link_latency,
                                weight=3))
                    link_count += 1

        ####################
        #### xrwang connect cpu to interposer, 5 6 9 10
        ####################
        cpu_node_list = [5, 6, 9, 10]
        for i in range(num_cpu):
            int_chiplet_interposer_links.append(
                            IntLink(link_id=link_count,
                                src_node=noi_routers[cpu_node_list[i]],
                                dst_node=noc_routers[num_GPU_routers+i],
                                src_outport="NoI",
                                dst_inport="NoC",
                                latency = link_latency,
                                weight=3))
            link_count += 1

        for i in range(num_cpu):
            int_chiplet_interposer_links.append(
                            IntLink(link_id=link_count,
                                dst_node=noi_routers[cpu_node_list[i]],
                                src_node=noc_routers[num_GPU_routers+i],
                                src_outport="NoC",
                                dst_inport="NoI",
                                latency = link_latency,
                                weight=3))
            link_count += 1


        network.int_links = (int_noc_links + int_noi_links
                + int_chiplet_interposer_links)
        print('#internal_links:', len(network.int_links),
                '   #NoC_links:', len(int_noc_links),
                '   #NoI_links:', len(int_noi_links),
                '   #chiplet_interposer_links:',
                    len(int_chiplet_interposer_links))

    # Register nodes with filesystem
    def registerTopology(self, options):
        for i in range(options.num_cpus):
            FileSystemConfig.register_node(
                [i], MemorySize(options.mem_size) // options.num_cpus, i
            )
