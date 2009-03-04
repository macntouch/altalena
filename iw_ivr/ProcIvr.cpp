/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "StdAfx.h"
#include "ProcIvr.h"
#include "CallWithDirectRtp.h"
#include "ProcScriptRunner.h"


namespace ivrworx
{

ProcIxMain::ProcIxMain(IN LpHandlePair pair, IN Configuration &conf)
:LightweightProcess(pair,IVR_Q,	__FUNCTION__),
_conf(conf),
_sipStackData(conf.IvrCnxInfo())
{
}



ProcIxMain::~ProcIxMain(void)
{
}

void
ProcIxMain::real_run()
{

	FUNCTRACKER;

	START_FORKING_REGION;
	
	//
	// Start SIP stack process
	//
	DECLARE_NAMED_HANDLE_PAIR(stack_pair);
	_stackPair = stack_pair;
	FORK(SipStackFactory::CreateSipStack(stack_pair,_conf));
	if (IW_FAILURE(WaitTillReady(Seconds(5),stack_pair)))
	{
		return;
	};


	I_AM_READY;

	HandlesList list;
	int index = 0;
	
	list.push_back(stack_pair.outbound);
	const int stack_index = index++;

	list.push_back(_inbound);
	const int inbound_index = index++;

	
	//
	// Message Loop
	// 
	BOOL shutdown_flag = FALSE;
	IwMessagePtr event;
	while(shutdown_flag == FALSE)
	{
		IX_PROFILE_CHECK_INTERVAL(25000);
		
		int index = -1;
		ApiErrorCode err_code = 
			SelectFromChannels(
			list,
			Seconds(60), 
			index, 
			event);

		if (err_code == API_TIMEOUT)
		{
			LogInfo("IxMain keep alive.");
			continue;
		}

		if (index == stack_index)
		{
			shutdown_flag = ProcessStackMessage(event, forking);
		} 
		else if (index == inbound_index)
		{
			shutdown_flag = ProcessInboundMessage(event,forking);
		}

	}


	Shutdown(Time(Seconds(5)),stack_pair);
	

	if (event != NULL_MSG && 
		event->message_id == MSG_PROC_SHUTDOWN_REQ)
	{

		SendResponse(event,new MsgShutdownAck());
	}

	END_FORKING_REGION;

}

BOOL 
ProcIxMain::ProcessInboundMessage(IN IwMessagePtr event, IN ScopedForking &forking)
{
	FUNCTRACKER;
	switch (event->message_id)
	{
	case MSG_PROC_SHUTDOWN_REQ:
		{
			return TRUE;
		}
	default:
		{
			BOOL oob_res = HandleOOBMessage(event);
			if (oob_res = FALSE)
			{
				LogCrit("Unknown message received id=[" << event->message_id_str << "]");
				throw;
			}
		}
	}

	return FALSE;

}

BOOL 
ProcIxMain::ProcessStackMessage(IN IwMessagePtr ptr, IN ScopedForking &forking)
{
	FUNCTRACKER;

	switch (ptr->message_id)
	{
	case MSG_CALL_OFFERED:
		{
			
			shared_ptr<MsgCallOfferedReq> call_offered = 
				shared_polymorphic_cast<MsgCallOfferedReq> (ptr);

			DECLARE_NAMED_HANDLE_PAIR(script_runner_handle);

			FORK_IN_THIS_THREAD(
				new ProcScriptRunner(
					_conf,                // configuration
					call_offered,		  // original message
					_stackPair,			  // handle to stack
					script_runner_handle) // handle created by stack for events
				);

			break;

		}
	case MSG_PROC_SHUTDOWN_REQ:
		{
			return TRUE;
			break;
		}
	default:
		{
			
		}
	}

	return FALSE;

}

}


