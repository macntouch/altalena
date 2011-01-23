require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")


dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 0
a=rtpmap:0 PCMU/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};
caller1 = sipcall:new();
caller1:makecall{dest="sip:24001@192.168.100.164",sdp=r1:localoffer()} 
r1:modify{sdp=caller1:remoteoffer()}

--
-- set up second media call
--
r2 = rtpproxy:new()
r2:allocate{sdp=dummyoffer};
caller2 = sipcall:new()
caller2:makecall{dest="sip:24002@192.168.150.144",sdp=r2:localoffer()} 
r2:modify{sdp=caller2:remoteoffer()}

--
-- half duplex the calls
--
r1:bridge{other=r2}

caller1:waitforhangup();
caller2:hangup();








