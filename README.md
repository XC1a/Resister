# Resister: A Resilient Interposer Architecture for Chiplet to Mitigate Timing Side-Channel Attacks

# 1. Introduction
This repository contains the code for the implementation of Resister based on the gem5 simulator, which is a secure interposer architecture for chiplet system, aiming to mitigate the timing side-channel attacks for the chiplet. There are mainly two stages: (1) detecting the potentially malicious packets and then (2) detouring the identified packets to deter the attacker, who relies on the packet latency to steal critical information. More details you can find in the paper published in an ACM journal:

 Xinrui Wang, Lang Feng, Yujie Wang, Taotao Xu, Yinhe Han, Zhongfeng Wang [Resister: A Resilient Interposer Architecture for Chiplet to Mitigate Timing Side-Channel Attacks](https://dl.acm.org/doi/10.1145/3748258), *ACM Trans. Des. Autom. Electron. Syst.*.

 Contact: Xinrui Wang ([xrwang@smail.nju.edu.cn](mailto:xrwang@smail.nju.edu.cn)) and Lang Feng ([fenglang3@mail.sysu.edu.cn](mailto:fenglang3@mail.sysu.edu.cn)).
 Please feel free to reach out if you have any questions!

 Before we get started, there are a few quick reminders for the user: (1) read the [gem5 instructions](https://www.gem5.org/getting_started/) and build the essential environment of gem5, (2) read the [garnet document](https://www.gem5.org/documentation/general_docs/ruby/) because the chiplet interconnector is based on it.

# 2. Repository Contents
Let us have a look at the directory of this repository.
### Gem5 Category
1. Folders `build_opts`, `build_tools`, `include`, `site_scons`, `system`, `util`, and `tests` are all related to gem5 basic components source and quite irrelevant to the function of Resister. So, you do not need to focus on these folders.
2. Folder `configs` is very important to configure the gem5 runtime behaviors (details of gem5 python configuration shells can be found on the gem5 website). The configuration files Resister needs to use are located in `configs/my/`.
3. Folder `src` contains the gem5 source code. The realization of Resister can be found in `src/mem/ruby/network/garnet`. Primarily, the input structure, `InputUnit.cc`, and the router unit, `RoutingUnit.cc`, are modified for Resister.
### Scripts and Relevant Files
4. Folder `resultsRecord` records the latency information and cache traces, which will be further introduced later.
5. Folder `scripts` contains the scripts and shells to evaluate the Resister, e.g., running benchmarks and running kinds of attacks.
6. Folder `cProgram` contains the source code for RSA, ECC, and crypto microbenchmark.
7. File `build.sh` and `buildx86O3.sh` help the user quickly construct the gem5 executable files, providing different options that will be introduced later.
8. File `testForTrace.sh` is used to get the cache access traces on a classical x86 system of different vulnerable programs. For instance, if we want to get the RSA cache access, this file needs to be used.
9. File `testForTraceChiplet.sh` takes the responsibility to simulate the trace on the chiplet system.

# 3. How to Build Your Gem5
Before you get started, please follow the instructions on gem5 website and install basic dependencies. The construction flow is based on `build.sh` and `buildx86O3.sh`. The difference between the two shells is the gem5 simulation mode: 
- `build.sh` constructs the simple structure named *standalone* mode that only focuses on the packet transactions in the chiplet network. This mode is used in our project to (1) analyze the attack, (2) evaluate the defense efficiency, and (3) evaluate the detection latency.
- `buildx86O3.sh` constructs the two-level cache CPU structure that can simulate the C programs. With the configuration of garnet, we can run benchmarks on this structure. This mode is used in the following scenarios: (1) capture the cache access traces, (2) run various  kinds of benchmarks.

The two scripts share the same options, which help the user to quickly construct the corresponding structures. 
- The first argument is `--policy [ori, resister, latency, valiant, TDMA]`, where the user can build the different defense strategies mentioned in the paper. To be brief, `ori` means the chiplet system has no any defense; `resister/latency` means the chiplet is Resister (the option `latency` is used to measure the detection latency because it will exit the program after decting the first malicous pakcet); `valiant` means the chiplet system will take valiant routing algorithm (will randomly detour all the packets and please refer to appendix the paper); `TDMA` means every chiplet in the system will send the packet as the round-robin approach, i.e., the `RPSL` policy dicussed in the paper (similarly, please refer to the appendix of the paper for details).
- The second argument is `--percent [0-1]`, which only works under `--policy resister/latency`. This option will set the detouring rate, i.e., the percentage of identified malicious packets that should be detoured.
-The third argument is `--coreNum`, which only works under `--policy TDMA`. Please set this option to the right value when simulating the TDMA.

After running the script successfully, you can see the executable files in `build/`. Please be careful whenever you need to change a chiplet system structure and remember to rebuild it.


# 3. How to Analyze the Timing Side-Channel Attack on Chiplet System
## 3.1 Capture the Cache Access Trace on Conventional CPU
The side-channel attack we focus on is caused by the secret program vulnerability: the key bit varying will lead to cache access pattern changing. Therefore, we first capture the cache access trace and then move it to the chiplet systems. We have already captured the traces record in the folder `resultsRecord/`, including `ECC-Cache-Trace-0/1`, `RSA-Cache-Trace-0/1`, where the 0/1 represents the key bit value. The idea of capturing the trace is to insert the `nop` instruction into the program, and then monitor this status to enable/disable the recording of the cache access. This part of the code is in `gem5-hetero-garnet/src/cpu/o3/decode.cc`.
If the user wants to re-capture the trace or obtain other programs' traces, there could be fthe ollowing operations:
1. Run the shell `./buildx86O3.sh --policy ori` to construct the classical CPU structure.
2. Enter the folder `cProgram` or move your new program repo into this folder. If you add the programs, you can follow the following RSA example to insert the `nop` instructions. This part of the modification is in `cProgram/libgcrypt-1.5.2/mpi/mpi-pow.c`. The modification of ECC programs is in `cProgram/ECC/micro-ecc-vulnerable/uECC.c`.
3. Then, enter `libgcrypt-1.5.2` for RSA or `ECC/micro-ecc-vulnerable` for ECC, then please execute `mkdir exeRepo0` and `mkdir exeRepo1` to store the execution files.
4. Run the shell `./build_trace.sh 0`, to get 1000 traces for the key bit being 0. Then, change the source code of the program to manually set the key bit being 1 to trigger the operations. Then, run the same shell `./build_trace.sh 1` again to get the 1000 traces for key bit being 1.
5. Based on the execution files, enter the folder `scripts` of this repo and run the shell `getCacheTrace.sh` to obtain the cache traces, which will be stored in `resultsRecord`. You need to slightly modify this shell, according to your programs (it is easy to do).

## 3.2 Simulate Traces as Packet Transactions on Chiplet System
After getting the traces, we can conduct the attack analysis and the defense efficiency. This part concentrates on the packet transferring behaviours of the chiplet system, so we use the *standalone* mode. So, in this part, you will learn how to evaluate the attack under different configurations.
1. Run the shell `build.sh`, you can choose the configurations based on Section 2 of this documentation.
2. Enter the folder `scripts`, and run the shell `./runAttack.sh --prefix [the prefix of latency file folders] --routing [routing settings]`. This shell will execute all the attack patterns mentioned in the paper on the chiplet system (you can read the annotation of this shell for details). You should focus on the `--prefix` that decides the prefix of the file folders in the `resultsRecord`. You also need to carefully set the value of `--routing`, where **1 means XY-routing** (ori and TDMA(RPSL) should choose this one), **6 means Resister's detouring algorithm** (resister needs to choose this one), and **7 means Valiant algorithm**.
3. After that, you will find a lot of latency files in `resultsRecord` folder. Then, use the python script `scripts/SVM_Sequence.py` to train and measure the accuracy rate of the attacks. You can run `python3 SVM.py --prefix ../resultsRecord/[the same prefix of the former step] -algo RSA` to get the results of ATT-40 ~ ATT-120, ATT-V1, ATT-V2, and ATT-V3 (to be clearer, the prefix is used to identify the latency files). Then, you can run `python3 SVM.py --Sprefix ../resultsRecord/log-[the special single attack folder] -algo RSA/ECC` to obtain the single result of Dynamic-Attack, Colluding-Attack, and the ECC Attack accuracy.

Now, you've got everything to evaluate the attacks under different configurations!

# 4. How to Run Benchmarks
## 4.1 Preparation
Before you get started, the project of SPEC 2017 and PARSEC should be prepared. I recommend you directly download the benchmark source code and put it into your user root, i.e., `/home/user_name`. Then, you also need to follow the instructions of these benchmarks to build the executable files. In terms of crypto microbenchmark, I have already put it in `cProgram`, for convenience (you can enter the folder and execute `python3 compile.py` to get the executable files).

Then you need to run the shell `buildx86O3.sh`, the options are already introduced in Section 2. After that, enter the folder `scripts` and `export UseP=/home/user_name`. To run the benchmarks, you can enter `scripts/SPEC` or `scripts/PARSEC` or `scripts/crypto` and run `./runAll.sh --routing [introduced before] --mesh [row of mesh] --split [control the simulated instructions]`. In terms of `--mesh`, it represents the mesh size, e.g., if you choose an 8x8 mesh, you need to configure this value to 8. In terms of `--split`, it represents the downsize factor of the simulated instruction and you could control this one when enlarging the mesh to ensure the simulation time is acceptable (otherwise, you need to wait a lot of days for a single case).

Ultimately, you can find the results in `scripts/[benchmark_name]/record`.

# 5. Conclusion
Here, this journey comes to an end. This work is just based on a very ideal consideration. Please feel free to contact!
