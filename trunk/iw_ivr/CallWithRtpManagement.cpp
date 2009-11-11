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
#include "CallWithRtpManagement.h"

using namespace boost;

namespace ivrworx
{
	CallWithRtpManagement::CallWithRtpManagement(
		IN Configuration &conf,
		IN ScopedForking &forking)
		:Call(forking),
		_conf(conf),
		_mrcpEnabled(FALSE),
		_rtspEnabled(FALSE),
		_rtspSession(forking),
		_mrcpSession(forking),
		_ringTimeout(Seconds(60))
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		_ringTimeout = Seconds(_conf.GetInt("ring_timeout"));
		
	}

	CallWithRtpManagement::CallWithRtpManagement(
		IN Configuration &conf,
		IN ScopedForking &forking,
		IN shared_ptr<MsgCallOfferedReq> offered_msg)
		:Call(forking,offered_msg),
		_conf(conf),
		_mrcpEnabled(FALSE),
		_rtspEnabled(FALSE),
		_origOffereReq(offered_msg),
		_rtspSession(forking),
		_mrcpSession(forking),
		_ringTimeout(Seconds(60))
	{
		_mrcpEnabled = _conf.GetBool("mrcp_enabled");
		_rtspEnabled = _conf.GetBool("rtsp_enabled");
		_ringTimeout = Seconds(_conf.GetInt("ring_timeout"));
		
		RemoteMedia(offered_msg->remote_media);
	}

	ApiErrorCode 
	CallWithRtpManagement::AcceptInitialOffer()
	{
		FUNCTRACKER;

		if (_callState != CALL_STATE_INITIAL_OFFERED)
		{
			LogWarn("CallWithRtpManagement::AcceptInitialOffer -  wrong call state:" << _callState << ", iwh:" << _stackCallHandle);
			return API_WRONG_STATE;
		}
		
		MediaFormatsList accepted_media_formats;
		MediaFormat speech_media_format = MediaFormat::PCMU;

		ApiErrorCode res = this->NegotiateMediaFormats(
			_origOffereReq->offered_codecs, 
			accepted_media_formats, 
			speech_media_format);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;
		};


		LogDebug("CallWithRtpManagement::AcceptInitialOffer -  *** allocating caller rtp ***");
		res = _callerRtpSession.Allocate(_origOffereReq->remote_media, speech_media_format);
		if (IW_FAILURE(res))
		{
			RejectCall();
			return res;

		}

		LogDebug("CallWithRtpManagement::AcceptInitialOffer - caller rtp = rtph:" << _callerRtpSession.RtpHandle() 
			<< ", lci:" << _callerRtpSession.LocalCnxInfo());

		

		if (_rtspEnabled == TRUE)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - *** allocating streamer rtp and session ***");
			res = _rtspRtpSession.Allocate(CnxInfo::UNKNOWN,speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			//
			// streamer session
			//
			res = _rtspSession.Allocate(
				_rtspRtpSession.LocalCnxInfo(),
				speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;
			}

			LogDebug("CallWithRtpManagement::AcceptInitialOffer - streaming info = imsh:" << _rtspSession.SessionHandle()
				<< ", associated rtsph:" << _rtspRtpSession.RtpHandle());

		}
	

		//
		// allocate mrcp session
		//
		if (_mrcpEnabled)
		{
			LogDebug("CallWithRtpManagement::AcceptInitialOffer *** allocating mrcp rtp connection and session ***");
			res = _mrcpRtpSession.Allocate(CnxInfo::UNKNOWN,speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			res = _mrcpSession.Allocate(
				_mrcpRtpSession.LocalCnxInfo(),
				speech_media_format);
			if (IW_FAILURE(res))
			{
				RejectCall();
				return res;

			}

			LogDebug("CallWithRtpManagement::AcceptInitialOffer - mrcp info = mrcph:" << _mrcpSession.SessionHandle()
				<< ", associated rtsph:" << _mrcpRtpSession.RtpHandle());
		}
		
		
		
		res = Call::AcceptInitialOffer(
			_callerRtpSession.LocalCnxInfo(),
			accepted_media_formats,
			speech_media_format);


		if (IW_SUCCESS(res))
		{
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - ims("  <<  _rtspSession.SessionHandle() << ")---> rtp(" << _rtspRtpSession.RtpHandle() << ") + ");
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - rtp("  <<  _callerRtpSession.RtpHandle()  << ")---> caller(" <<  StackCallHandle() << ") " << RemoteMedia());
			LogDebug("CallWithRtpManagement::AcceptInitialOffer - mrcp(" <<  _mrcpSession.SessionHandle() << ")---> rtp(" << _mrcpRtpSession.RtpHandle() << ") + ");
		}
		

		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::PlayFile(IN const string &file_name, IN BOOL sync, IN BOOL loop)
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			LogDebug("CallWithRtpManagement::PlayFile - Media streaming disabled for iwh:" << _stackCallHandle )
			return API_FAILURE;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		_rtspSession.StopPlay();
		
		ApiErrorCode res = _rtspRtpSession.Bridge(_callerRtpSession);
		if (IW_FAILURE(res))
		{
			return res;
		}
	
		

		res = _rtspSession.PlayFile(file_name, sync, loop);

		return res;

	}
	ApiErrorCode 
	CallWithRtpManagement::Speak(IN const string &mrcp_body, BOOL sync )
	{
		FUNCTRACKER;

		if (!_mrcpEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;

		}

		_mrcpSession.StopSpeak();

		ApiErrorCode res = _mrcpRtpSession.Bridge(_callerRtpSession);
		if (IW_FAILURE(res))
		{
			return res;
		}

		res = _mrcpSession.Speak(mrcp_body, sync);
		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::StopSpeak()
	{
		FUNCTRACKER;

		if (!_mrcpEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = _mrcpSession.StopSpeak();

		return res;

	}

	ApiErrorCode 
	CallWithRtpManagement::StopPlay()
	{
		FUNCTRACKER;

		if (!_rtspEnabled)
		{
			return API_FEATURE_DISABLED;
		}

		if (_callState != CALL_STATE_CONNECTED)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res =  _rtspSession.StopPlay();

		return res;


	}


	void 
	CallWithRtpManagement::UponCallTerminated(IwMessagePtr ptr)
	{
		_rtspSession.InterruptWithHangup();
		_mrcpSession.InterruptWithHangup();

		Call::UponCallTerminated(ptr);
	}

	ApiErrorCode 
	CallWithRtpManagement::MakeCall(IN const string &destination_uri)
	{
		
		FUNCTRACKER;

		if (_callState != CALL_STATE_UNKNOWN)
		{
			return API_WRONG_STATE;
		}

		ApiErrorCode res = API_SUCCESS;

		LogDebug("CallWithRtpManagement::MakeCall *** allocating caller rtp ***");

		//
		// Allocate rtp connection of the caller.
		//
		res = _callerRtpSession.Allocate();
		if (IW_FAILURE(res))
		{
			return res;
		}
		LogDebug("CallWithRtpManagement::MakeCall - caller rtp = rtph:" << _callerRtpSession.RtpHandle() 
			<< ", lci:" << _callerRtpSession.LocalCnxInfo());

		
		
		LogDebug("CallWithRtpManagement::MakeCall *** calling ***");
		res = Call::MakeCall(destination_uri,_callerRtpSession.LocalCnxInfo(),_ringTimeout);
		if (IW_FAILURE(res))
		{
			return res;
		};

		res = _callerRtpSession.Modify(RemoteMedia(),_acceptedSpeechFormat);
		if (IW_FAILURE(res))
		{
			return res;
		};

		LogDebug("CallWithRtpManagement::MakeCall - caller remote rtp = iwh:" <<  _stackCallHandle 
			<< ", rci:" << RemoteMedia() 
			<< " codec:" << _acceptedSpeechFormat);


		//
		// Allocate rtsp session
		//
		if (_rtspEnabled)
		{
			//
			// rtp
			//
			LogDebug("CallWithRtpManagement::MakeCall *** allocating streamer rtp and session ***");
			res = _rtspRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			//
			// streamer session
			//
			res = _rtspSession.Allocate(
				_rtspRtpSession.LocalCnxInfo(), 
				_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			};

			LogDebug("CallWithRtpManagement::MakeCall - streaming info = imsh:" << _rtspSession.SessionHandle()
				<< ", associated rtsph:" << _rtspRtpSession.RtpHandle());
		};



		//
		// allocate mrcp session
		//
		if (_mrcpEnabled)
		{
			LogDebug("CallWithRtpManagement::MakeCall *** allocating mrcp rtp connection and session ***");
			res = _mrcpRtpSession.Allocate(CnxInfo::UNKNOWN,_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			res = _mrcpSession.Allocate(_mrcpRtpSession.LocalCnxInfo(),_acceptedSpeechFormat);
			if (IW_FAILURE(res))
			{
				return res;
			}

			LogDebug("CallWithRtpManagement::MakeCall - mrcp info = mrcph:" << _mrcpSession.SessionHandle()
				<< ", associated rtsph:" << _mrcpRtpSession.RtpHandle());
			

		}


		LogDebug("CallWithRtpManagement::MakeCall ims("  <<  _rtspSession.SessionHandle() << ")---> rtp(" << _rtspRtpSession.RtpHandle() << ") + ");
		LogDebug("CallWithRtpManagement::MakeCall rtp("  <<  _callerRtpSession.RtpHandle()  << ")---> caller(" <<  StackCallHandle() << ") " << RemoteMedia());
		LogDebug("CallWithRtpManagement::MakeCall mrcp(" <<  _mrcpSession.SessionHandle() << ")---> rtp(" << _mrcpRtpSession.RtpHandle() << ") + ");
		
		return API_SUCCESS;

	}

	CallWithRtpManagement::~CallWithRtpManagement(void)
	{
		FUNCTRACKER;

		_rtspSession.TearDown();
		_mrcpSession.TearDown();
		_callerRtpSession.TearDown();
		_mrcpRtpSession.TearDown();
		_rtspRtpSession.TearDown();


	}

}
