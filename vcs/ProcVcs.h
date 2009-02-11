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

#pragma once

#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "CallWithRTPManagment.h"

using namespace std;

namespace ivrworx
{


	class ProcIxMain :
		public LightweightProcess
	{

	public:

		ProcIxMain(IN LpHandlePair pair, IN CcuConfiguration &conf);

		virtual void real_run();

		virtual ~ProcIxMain(void);

	protected:

		BOOL ProcessStackMessage(
			IN IxMsgPtr event,
			IN ScopedForking &forking
			);

		BOOL ProcessInboundMessage(
			IN IxMsgPtr event,
			IN ScopedForking &forking
			);

		void StartScript(
			IN IxMsgPtr msg);

	private:

		LpHandlePair _stackPair;

		CnxInfo _sipStackData;

		CcuConfiguration &_conf;

	};

	typedef 
		shared_ptr<CLuaVirtualMachine> CLuaVirtualMachinePtr;

	class ProcScriptRunner
		: public LightweightProcess

	{
	public:

		ProcScriptRunner(
			IN CcuConfiguration &conf,
			IN IxMsgPtr msg, 
			IN LpHandlePair stack_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL HandleOOBMessage(IN IxMsgPtr msg);

	private:

		IxMsgPtr _initialMsg;

		LpHandlePair _stackPair;

		CcuConfiguration &_conf;

		int _stackHandle;

	};

	class IxScript : 
		public CLuaScript
	{
	public:

		IxScript(IN CLuaVirtualMachine &_vmPtr, IN CallWithRtpRelay &_callSession);

		~IxScript();

	private:

		// When the script calls a class method, this is called
		virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber) ;

		// When the script function has returns
		virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

		int LuaAnswerCall(CLuaVirtualMachine& vm);

		int LuaHangupCall(CLuaVirtualMachine& vm);

		int LuaWait(CLuaVirtualMachine& vm);

		CallWithRtpRelay &_callSession;

		int _methodBase;

		CLuaVirtualMachine &_vmPtr;

	};

}




