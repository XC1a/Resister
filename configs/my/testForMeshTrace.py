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
#
# Author: Tushar Krishna

import m5
from m5.objects import *
from m5.defines import buildEnv
from m5.util import addToPath
import os, argparse, sys, random

addToPath("../")

from common import Options
from ruby import Ruby

# xrwang Test Vector
SampleT = 50
testCase = 1


# Get paths we might need.  It's expected this file is in m5/configs/example.
config_path = os.path.dirname(os.path.abspath(__file__))
config_root = os.path.dirname(config_path)
m5_root = os.path.dirname(config_root)

parser = argparse.ArgumentParser()
Options.addNoISAOptions(parser)

##################

parser.add_argument(
    "--sampleT",
    type=int,
    default=50,
    help="Number of digits of precision after decimal point\
                        for injection rate",
)

parser.add_argument(
    "--whiteND",
    type=int,
    default=0,
    help="If user adds the white noise\
                        use this param to set deviation",
)

parser.add_argument(
    "--whiteND_LOW",
    type=int,
    default=0,
    help="If user adds the white noise\
                        use this param to set deviation of low boundary",
)

parser.add_argument(
    "--testCase",
    type=int,
    default=1,
    help="0 random injection\
            1 test RSA attack, 2 theory attack",
)

parser.add_argument(
    "--randomPNum",
    type=int,
    default=1000,
    help="every node insert packet number",
)
##################

parser.add_argument(
    "--synthetic",
    default="uniform_random",
    choices=[
        "uniform_random",
        "tornado",
        "bit_complement",
        "bit_reverse",
        "bit_rotation",
        "neighbor",
        "shuffle",
        "transpose",
    ],
)

parser.add_argument(
    "-i",
    "--injectionrate",
    type=float,
    default=0.1,
    metavar="I",
    help="Injection rate in packets per cycle per node. \
                        Takes decimal value between 0 to 1 (eg. 0.225). \
                        Number of digits after 0 depends upon --precision.",
)

parser.add_argument(
    "--precision",
    type=int,
    default=3,
    help="Number of digits of precision after decimal point\
                        for injection rate",
)

parser.add_argument(
    "--sim-cycles", type=int, default=1000, help="Number of simulation cycles"
)

parser.add_argument(
    "--num-packets-max",
    type=int,
    default=-1,
    help="Stop injecting after --num-packets-max.\
                        Set to -1 to disable.",
)

parser.add_argument(
    "--single-sender-id",
    type=int,
    default=-1,
    help="Only inject from this sender.\
                        Set to -1 to disable.",
)

parser.add_argument(
    "--single-dest-id",
    type=int,
    default=-1,
    help="Only send to this destination.\
                        Set to -1 to disable.",
)

# *Taotao Xu*
# parser.add_argument(
#    "--single-send1-id",
#    type=int,
#    default=-1,
#    help="Only inject from this sender.\
#                        Set to -1 to disable.",
# )
#
# parser.add_argument(
#    "--single-dest1-id",
#    type=int,
#    default=-1,
#    help="Only send to this destination.\
#                        Set to -1 to disable.",
# )
# *Taotao Xu*

parser.add_argument(
    "--inj-vnet",
    type=int,
    default=-1,
    choices=[-1, 0, 1, 2],
    help="Only inject in this vnet (0, 1 or 2).\
                        0 and 1 are 1-flit, 2 is 5-flit.\
                        Set to -1 to inject randomly in all vnets.",
)

parser.add_argument(
    "--TraceFile", type=str, default="", help="Trace file to simulate"
)


# TraceFile = Param.String("", "file to simulate")
#
# Add the ruby specific and protocol specific options
#
Ruby.define_options(parser)

args = parser.parse_args()

# *Taotao Xu*
default_num_packets_max = 0
SampleT = args.sampleT
whiteND = args.whiteND
whiteND_LOW = args.whiteND_LOW
if whiteND != 0:
    assert whiteND < SampleT
    assert whiteND_LOW < whiteND

whiteND = args.whiteND - whiteND_LOW
print("Now sample T is ", SampleT)
print("Now sample nosie", whiteND, whiteND_LOW)

testCase = args.testCase

cpus = []
for i in range(args.num_cpus):
    cpu = GarnetSyntheticTraffic(
        num_packets_max=default_num_packets_max,
        single_sender=args.single_sender_id,
        single_dest=args.single_dest_id,
        # single_send1=args.single_send1_id,  # append for test
        # single_dest1=args.single_dest1_id,  # append for test
        sim_cycles=args.sim_cycles,
        traffic_type=args.synthetic,
        inj_rate=args.injectionrate,
        inj_vnet=args.inj_vnet,
        precision=args.precision,
        num_dest=args.num_dirs,  #!!! note this is to symbol the number of destinations, not the "number" of a destination
    )
    cpus.append(cpu)


# for mesh 4*4
if (
    testCase == 0
):  # test for the influence of defense algorithm under the randmoly accesses
    for cpu in cpus:
        cpu.inj_rate = args.injectionrate
        cpu.inj_vnet = 2
        cpu.num_packets_max = 1000000000
        cpu.sim_cycles = 2000000000 # follow modular chiplet routing


elif testCase == 1:
    cpus[6].inj_rate = 0
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 0
    cpus[6].rule_period_cycle = SampleT
    cpus[6].num_packets_max = 500
    cpus[6].whiteND = whiteND
    cpus[6].whiteND_LOW = whiteND_LOW
    # cpus[6].num_packets_max = 0

    cpus[2].inj_rate = 1
    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile
elif testCase == 2:
    i = 0
    for cpu in cpus:
        dst = 0
        while dst == i:
            dst = random.randint(0, 15)
        cpu.single_dest = dst
        cpu.packet_rule = 1
        cpu.LoopT = random.randint(0, 100)
        cpu.Func1T = random.randint(0, 16)
        cpu.Func1Itl = random.randint(2, 8)
        cpu.Func2T = random.randint(0, 16)
        cpu.Func2Itl = random.randint(0, 8)
        cpu.LOOPItl = random.randint(10, 300)
        cpu.FItl = random.randint(2, 30)
        cpu.num_packets_max = 10000
        cpu.sim_cycles = 100000000  # need to set. cannot be set to -1
        i = i + 1
elif testCase == 3:
    cpus[6].inj_rate = 0
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 3
    cpus[6].rule_period_cycle = SampleT
    cpus[6].num_packets_max = 500
    cpus[6].whiteND = whiteND
    cpus[6].whiteND_LOW = whiteND_LOW
    # cpus[6].num_packets_max = 0

    cpus[2].inj_rate = 1
    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile
elif testCase == 4:
    cpus[6].inj_rate = 0
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 4
    cpus[6].rule_period_cycle = SampleT
    cpus[6].num_packets_max = 500 + 2 * 500
    cpus[6].whiteND = whiteND
    cpus[6].whiteND_LOW = whiteND_LOW
    # cpus[6].num_packets_max = 0

    cpus[2].inj_rate = 1
    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile
elif testCase == 5:
    # col 2
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 0
    cpus[6].rule_period_cycle = 50
    cpus[6].num_packets_max = 200
    cpus[6].whiteND = 0
    cpus[6].whiteND_LOW = 0
    cpus[6].FItl = 10500 * 2

    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile
    cpus[2].FItl = 10500 * 2

    # col 0
    cpus[4].single_dest = 8
    cpus[4].packet_rule = 0
    cpus[4].rule_period_cycle = 50
    cpus[4].num_packets_max = 200
    cpus[4].whiteND = 0
    cpus[4].whiteND_LOW = 0
    cpus[4].FItl = 10500 * 0

    cpus[0].single_dest = 12
    cpus[0].packet_rule = 2
    cpus[0].num_packets_max = 10000
    cpus[0].TraceFile = args.TraceFile
    cpus[0].FItl = 10500 * 0

    # col 1
    cpus[5].single_dest = 9
    cpus[5].packet_rule = 0
    cpus[5].rule_period_cycle = 50
    cpus[5].num_packets_max = 200
    cpus[5].whiteND = 0
    cpus[5].whiteND_LOW = 0
    cpus[5].FItl = 10500 * 1

    cpus[1].single_dest = 13
    cpus[1].packet_rule = 2
    cpus[1].num_packets_max = 10000
    cpus[1].TraceFile = args.TraceFile
    cpus[1].FItl = 10500 * 1

    # col 3
    cpus[7].single_dest = 11
    cpus[7].packet_rule = 0
    cpus[7].rule_period_cycle = 50
    cpus[7].num_packets_max = 200
    cpus[7].whiteND = 0
    cpus[7].whiteND_LOW = 0
    cpus[7].FItl = 10500 * 3

    cpus[3].single_dest = 15
    cpus[3].packet_rule = 2
    cpus[3].num_packets_max = 10000
    cpus[3].TraceFile = args.TraceFile
    cpus[3].FItl = 10500 * 3
elif testCase == 6:
    cpus[6].inj_rate = 0
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 0
    cpus[6].rule_period_cycle = SampleT
    cpus[6].num_packets_max = 17000
    cpus[6].whiteND = whiteND
    cpus[6].whiteND_LOW = whiteND_LOW
    # cpus[6].num_packets_max = 0

    cpus[2].inj_rate = 1
    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile

elif testCase == 7:
    cpus[6].inj_rate = 0
    cpus[6].single_dest = 10
    cpus[6].packet_rule = 0
    cpus[6].rule_period_cycle = SampleT
    cpus[6].num_packets_max = 500
    cpus[6].whiteND = whiteND
    cpus[6].whiteND_LOW = whiteND_LOW
    # cpus[6].num_packets_max = 0
    cpus[4].inj_rate = 0
    cpus[4].single_dest = 8
    cpus[4].packet_rule = 0
    cpus[4].rule_period_cycle = SampleT
    cpus[4].num_packets_max = 500
    cpus[4].whiteND = whiteND
    cpus[4].whiteND_LOW = whiteND_LOW

    cpus[5].inj_rate = 0
    cpus[5].single_dest = 9
    cpus[5].packet_rule = 0
    cpus[5].rule_period_cycle = SampleT
    cpus[5].num_packets_max = 500
    cpus[5].whiteND = whiteND
    cpus[5].whiteND_LOW = whiteND_LOW

    cpus[7].inj_rate = 0
    cpus[7].single_dest = 11
    cpus[7].packet_rule = 0
    cpus[7].rule_period_cycle = SampleT
    cpus[7].num_packets_max = 500
    cpus[7].whiteND = whiteND
    cpus[7].whiteND_LOW = whiteND_LOW

    cpus[2].inj_rate = 1
    cpus[2].single_dest = 14
    cpus[2].packet_rule = 2
    cpus[2].num_packets_max = 10000
    cpus[2].TraceFile = args.TraceFile
    for cpu in cpus:
        cpu.sim_cycles = 2000000000 # follow modular chiplet routing
        cpu.response_limit = 50000000
# cpus[10].inj_rate = 0.3
# cpus[10].num_dest = 6
#
# for mesh 8*8
# cpus[37].inj_rate = 0.5
# cpus[37].dst_node = 45
# cpus[45].inj_rate = 0.5
# cpus[45].dst_node = 37
# cpus[13].dst_node = 61
# cpus[13].inj_rate = 0.5
# cpus[61].dst_node = 13
# functional incorrectly!!!!!!

# *Taotao Xu*

# cpus = [
#    GarnetSyntheticTraffic(
#        num_packets_max=args.num_packets_max,
#        single_sender=args.single_sender_id,
#        single_dest=args.single_dest_id,
#        sim_cycles=args.sim_cycles,
#        traffic_type=args.synthetic,
#        inj_rate=args.injectionrate,
#        inj_vnet=args.inj_vnet,
#        precision=args.precision,
#        num_dest=args.num_dirs,
#    )
#    for i in range(args.num_cpus)
# ]


# create the desired simulated system
system = System(cpu=cpus, mem_ranges=[AddrRange(args.mem_size)])


# Create a top-level voltage domain and clock domain
system.voltage_domain = VoltageDomain(voltage=args.sys_voltage)

system.clk_domain = SrcClockDomain(
    clock=args.sys_clock, voltage_domain=system.voltage_domain
)

Ruby.create_system(args, False, system)

# Create a seperate clock domain for Ruby
system.ruby.clk_domain = SrcClockDomain(
    clock=args.ruby_clock, voltage_domain=system.voltage_domain
)

i = 0
for ruby_port in system.ruby._cpu_ports:
    #
    # Tie the cpu test ports to the ruby cpu port
    #
    cpus[i].test = ruby_port.in_ports
    i += 1

# -----------------------
# run simulation
# -----------------------

root = Root(full_system=False, system=system)
root.system.mem_mode = "timing"

# Not much point in this being higher than the L1 latency
m5.ticks.setGlobalFrequency("1ps")
# 1ps --> 1ns

# instantiate configuration
m5.instantiate()

# simulate until program terminates
exit_event = m5.simulate(args.abs_max_tick)

print("Exiting @ tick", m5.curTick(), "because", exit_event.getCause())
