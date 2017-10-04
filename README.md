# NS-3 simulator for RDMA
This is an NS-3 simulator for RDMA over Converged Ethernet v2 (RoCEv2). It includes the implementation of DCQCN, TIMELY, PFC, ECN and Broadcom shared buffer switch.

It is based on NS-3 version 3.17, and ported to Visual Studio environment, as explained [here](https://www.nsnam.org/wiki/Ns-3_on_Visual_Studio_2012).  

## Note
TIMELY implementation is in "timely" branch and hasn't been merged into the master branch. So, you may not be able to simulate DCQCN and TIMELY simultaneously at this moment.

## Quick Start

### Build
To compile it out-of-the-box, you need Visual Studio *2015* (not 2013 or 2017). 
People have successfully built it with *free* version, 
which can be downloaded [here](https://imagine.microsoft.com/en-us/Catalog/Product/101). 
Open windows/ns-3-dev/ns-3-dev.sln, just build the whole solution.

If you cannot get a Windows machine or Visual Studio for any reason, you may try building it with the original Makefile. We have done it a while back, but now you probably need to edit a few things in waf to make it work.

### Run
The binary will be generated at windows/ns-3-dev/x64/Release/main.exe.
We include a sample configuration file at windows/ns-3-dev/x64/Release/mix/config.txt
Execute main.exe in windows/ns-3-dev/x64/Release/:
```
cd windows\ns-3-dev\x64\Release\
main.exe mix\config.txt
```

It runs a 2:1 incast at 40Gbps for 1 second. Please allow a few minutes for it to finish.
The trace will be generated at mix/mix.tr, as defined by mix/config.txt

There are quite a few options in mix/config.txt. We will gradually add documentation.
For your own convenience you can just check the code, 
project "main" -- source files -- "third.cc", and see how these options are parsed.
You can also raise issues if you have any questions.

## What did we add exactly?

**point-to-point/model/qbb-net-device.cc** and all other qbb-* files: 

DCQCN and PFC implementation.
It also includes go-back-to-N and go-back-to-0 that handle packet drop due to corruption.

In 2013, we got a very basic NS-3 PFC implementation somewhere, and developed based on it. 
We cannot find the original repository anymore.

**network/model/broadcom-node.cc** and **.h**: 

This implements a Broadcom ASIC switch model, which
is mostly doing all kinds of buffer threshold-related operations. These include deciding 
whether PFC should be triggered, ECN should be marked, buffer is too full so packets should
be dropped, etc. It supports both static and dynamic thresholds for PFC.

*Disclaim: this module is purely based on authors' personal understanding of Broadcom ASIC. It does not reflect any official confirmation from either Microsoft or Broadcom.*

**network/utils/broadcom-egress-queue.cc** and **.h**: 

This is the actual MMU buffering packets.
It also includes switch scheduler, i.e., when upper layer ask for a packet to send, it will
decide which queue to be dequeued. Strategies like strict priority and round robin are supported.

**applications/model/udp-echo-client.cc**: 

We implement the RDMA client here, which aligns 
with the fact that RoCEv2 includes UDP header. In particular, original UDP client has troubles
when PFC pause the link. Original UDP client keeps sending packets at line rate, soon
it builds up huge queue and memory runs out. Here we throttle the sending rate if it gets
pushed back by PFC.

**internet/model/seq-ts-header.cc** and **.h**: 

We didn't implement the full InfiniBand
header. Instead, what we really need is just the sequence number (for detecting corruption
drops, and also help us understand the throughput) and timestamp (required by TIMELY.) 
This is where we encode this information into packets.

**main/third.cc**: 

The main() function.

There may be other edits here and there, especially the trace generation is scattered
among various network stacks. But above are the major ones.

## Q&A

**Q: Why do you port it to Windows?**

A: This is a Microsoft project. Visual Studio, including the free version, works well.

**Q: Fine. What if I want to run it on Linux, and do not want to spend time changing the build process?**

A: You can build it using Visual Studio and run the .exe using WINE. We have tested WINE 1.6.2 and it works well.

**Q: I don't understand ... (some part of the code or configuration)**

A: Raise issues on GitHub, so that your questions can also help others. If you really do
not want others know you are working on this, you can email yibzh@microsoft.com

**Q: What papers should I cite, if I also publish?**

A: Below are the ones you should definitely check. They are ranked from most relevant to 
less. That said, all of them are quite relevant:

*ECN or Delay: Lessons Learnt from Analysis of DCQCN and TIMELY*, CoNEXT'16 (this project is released with this paper, we ask you to at least cite this paper if you use this code.)

*Congestion Control for Large-scale RDMA Deployments*, SIGCOMM'15 (DCQCN)

*TIMELY: RTT-based Congestion Control for the Datacenter*, SIGCOMM'15 (TIMELY)

*RDMA over Commodity Ethernet at Scale*, SIGCOMM'16 (discussed go-back-to-N)

*Deadlocks in Datacenter Networks: Why Do They Form, and How to Avoid Them*, HotNets'16 (PFC deadlock analysis, directly used this simulator.)
