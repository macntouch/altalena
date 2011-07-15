require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== SCRIPT START ===");

dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 0 101
a=rtpmap:0 PCMU/8000
a=rtpmap:101 telephone-event/8000
]]

---
--- allocate connection
---
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};

--
-- set up first media call
--
caller1 = sipcall:new();
caller1:makecall{dest="sip:24001@192.168.150.3",sdp=r1:localoffer()}
r1:modify{sdp=caller1:remoteoffer()}


---
--- set up streamer connection
---
---
r2 = rtpproxy:new();
r2:allocate{sdp=dummyoffer};

streamer1 = streamer:new();
streamer1:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE,sdp=r2:localoffer()}
r2:modify{sdp=streamer1:localoffer()}

r2:bridge{other=r1, duplex="full"};

streamer1:play{file="C:\\sounds\\greeting.wav"}

NO_INPUT_TIMEOUT = 20
timeout_val=NO_INPUT_TIMEOUT

while (true) do
    --
    -- wait for hangup or dtmf
    --
	wait_start = os.clock();
	res,i = iw.waitforevent{actors={r1,caller1,streamer1}, timeout=timeout_val};
	wait_end = os.clock();

	l:loginfo ("waitforevent -  res:"..res..", i:"..i);

	if (res == iw.API_TIMEOUT) then
		streamer1:play{file="C:\\sounds\\no_response.wav"};
		timeout_val = NO_INPUT_TIMEOUT;
	elseif (i==1) then
		dtb = r1:dtmfbuffer();
		if (dtb ~= "") then
			streamer1:play{file="C:\\sounds\\"..dtb:sub(1,1)..".wav"};
		end
		r1:cleandtmfbuffer();
		timeout_val = 20;
	elseif (i==2) then
		break;
	elseif (i==3) then
		timeout_val = NO_INPUT_TIMEOUT - (wait_end - wait_start);
		l:loginfo ("delta:"..(wait_end - wait_start));
		if (timeout_val <=0) then
			timeout_val = NO_INPUT_TIMEOUT
		end
	end

end


l:loginfo("=== SCRIPT END ===");

