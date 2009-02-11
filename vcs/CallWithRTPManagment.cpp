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

#include "CallWithRTPManagment.h"

namespace ivrworx
{

	CallWithRtpRelay::CallWithRtpRelay(
		IN LpHandlePair stack_pair,
		IN LightweightProcess &facade)
		:Call(stack_pair,facade),
		_callerRtpSession(facade),
		_imsRtpSession(facade),
		_imsSession(facade)
	{

	}

	CallWithRtpRelay::CallWithRtpRelay(
		IN LpHandlePair _stackPair, 
		IN int call_handle,
		IN CnxInfo offered_media,
		IN LightweightProcess &facade)
		:Call(_stackPair, call_handle, offered_media, facade),
		_callerRtpSession(facade),
		_imsRtpSession(facade),
		_imsSession(facade)
	{
	}


	CallWithRtpRelay::~CallWithRtpRelay(void)
	{
		FUNCTRACKER;
		_callerRtpSession.CloseRTPConnection();
	}

	IxApiErrorCode
		CallWithRtpRelay::MakeCall(IN const wstring &destination_uri)
	{
		FUNCTRACKER;

		IxApiErrorCode res = _callerRtpSession.AllocateRTPConnection();
		if (CCU_FAILURE(res))
		{
			LogWarn("Error allocating RTP connection res=[" << res << "]");
			return res;
		}

		res  = Call::MakeCall(destination_uri,_callerRtpSession.LocalMediaData());
		if (CCU_FAILURE(res))
		{
			LogWarn("Error Making call to destination_uri=[" << destination_uri << "],  res=[" << res << "]");
			return res;
		}

		_callerRtpSession.ModifyRTPConnection(RemoteMedia());
		if (CCU_FAILURE(res))
		{
			LogWarn("Error modifying RTP connection res=[" << res << "], hanging up the session");
			HagupCall();
			return res;
		}

		return CCU_API_SUCCESS;
	}

	IxApiErrorCode CallWithRtpRelay::AcceptCall()
	{
		FUNCTRACKER;
		IX_PROFILE_FUNCTION();

		IxApiErrorCode res = _callerRtpSession.AllocateRTPConnection();
		if (CCU_FAILURE(res))
		{
			return res;
		};

		res = _imsRtpSession.AllocateRTPConnection();
		if (CCU_FAILURE(res))
		{
			return res;
		};

//		_imsSession.AllocateIMSConnection()


			res = Call::AcceptCall(_callerRtpSession.LocalMediaData());
		return res;

	}

	IxApiErrorCode
		CallWithRtpRelay::PlayFile(IN const wstring &file_name)
	{
		FUNCTRACKER;

// 		// allocate Rtp Relay connection for Ims
// 		CcuRtpSession ims_rtp_session(_parentProcess);
// 		IxApiErrorCode res = ims_rtp_session.AllocateRTPConnection();
// 		if (CCU_FAILURE(res))
// 		{
// 			LogWarn("Error allocating RTP connection res=[" << res << "]");
// 			return res;
// 		}
// 
// 		// allocate Ims session
// 		ImsSession ims_session(_parentProcess);
// 		ims_session.AllocateIMSConnection(ims_rtp_session.LocalMediaData(),file_name);
// 		if (CCU_FAILURE(res))
// 		{
// 			LogWarn("Error allocating IMS connection res=[" << res << "]");
// 			return res;
// 		}
// 
// 		// though Ims doesn't care, update the remote end of its Rtp connection
// 		ims_rtp_session.ModifyRTPConnection(ims_session.ImsMediaData());
// 		if (CCU_FAILURE(res))
// 		{
// 			LogWarn("Error modifying RTP connection res=[" << res << "]");
// 			return res;
// 		}
// 
// 		// bridge caller connection
// 		res = _callerRtpSession.BridgeRTPConnection(ims_rtp_session);
// 		if (CCU_FAILURE(res))
// 		{
// 			LogWarn("Error bridging RTP connection res=[" << res << "]");
// 			return res;
// 		}

// 		res = ims_session.PlayFile(ims_rtp_session.LocalMediaData(),file_name);
// 		if (CCU_FAILURE(res))
// 		{
// 			LogWarn("Error Playing file=[" << file_name <<"] to=[" << ims_rtp_session.LocalMediaData() << "], res=[" << res << "]");
// 			return res;
// 		}

		return CCU_API_SUCCESS;

	}
}
