# Copyright (c) 2016 Georgia Institute of Technology
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

from m5.objects.ClockedObject import ClockedObject
from m5.params import *
from m5.proxy import *


class GarnetSyntheticTraffic(ClockedObject):
    type = "GarnetSyntheticTraffic"
    cxx_header = (
        "cpu/testers/garnet_synthetic_traffic/GarnetSyntheticTraffic.hh"
    )
    cxx_class = "gem5::GarnetSyntheticTraffic"

    block_offset = Param.Int(6, "block offset in bits")
    num_dest = Param.Int(1, "Number of Destinations")
    memory_size = Param.Int(65536, "memory size")
    sim_cycles = Param.UInt64(1000, "Number of simulation cycles")
    num_packets_max = Param.Int(
        -1,
        "Max number of packets to send. \
                        Default is to keep sending till simulation ends",
    )
    single_sender = Param.Int(
        -1,
        "Send only from this node. \
                                   By default every node sends",
    )
    single_dest = Param.Int(
        -1,
        "Send only to this dest. \
                                 Default depends on traffic_type",
    )

    # xrwang
    packet_rule = Param.Int(
        -1,
        "the default random transformation.",
    )
    rule_period_cycle = Param.Int(5, "self-generating rule cycles")

    whiteND = Param.Int(0, "white noise deviation")
    whiteND_LOW = Param.Int(0, "white noise deviation")

    LoopT = Param.Int(1, "Loop cnt")
    Func1T = Param.Int(4, "times of Func1")
    Func1Itl = Param.Int(2, "interval in func1")
    Func2T = Param.Int(5, " ")
    Func2Itl = Param.Int(2, " ")
    LOOPItl = Param.Int(10, " ")
    FItl = Param.Int(0, " ")

    ###### update in 6-24, simulate for access patterns
    TraceFile = Param.String("", "file to simulate")

    # xrwang

    # *Taotao Xu*
    # single_send1 = Param.Int(
    #    -1,
    #    "Send only from this node. \
    #                               By default every node sends",
    # )
    # single_dest1 = Param.Int(
    #    -1,
    #    "Send only to this dest. \
    #                             Default depends on traffic_type",
    # )

    # send and dest can handle multi parameters!
    # multi_sender = Param.List(
    #    [],
    #    default=[],
    #    desc="Send only from these nodes. \
    #                                                              By default, every nodes sends.",
    # )
    # multi_dest = Param.List(
    #    [],
    #    default=[],
    #    desc="Dest only from these nodes. \
    #                                                           By default, every nodes dest.",
    # )
    # multi_sender = [int(x) for x in multi_sender]
    # multi_dest = [int(x) for x in multi_dest]
    # *Taotao Xu*

    traffic_type = Param.String("uniform_random", "Traffic type")
    inj_rate = Param.Float(0.1, "Packet injection rate")
    inj_vnet = Param.Int(
        -1,
        "Vnet to inject in. \
                              0 and 1 are 1-flit, 2 is 5-flit. \
                                Default is to inject in all three vnets",
    )
    precision = Param.Int(
        3,
        "Number of digits of precision \
                              after decimal point",
    )
    response_limit = Param.Cycles(
        50000000,
        "Cycles before exiting \
                                            due to lack of progress",
    )
    test = RequestPort("Port to the memory system to test")
    system = Param.System(Parent.any, "System we belong to")
