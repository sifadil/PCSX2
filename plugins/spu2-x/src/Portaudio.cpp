/* SPU2-X, A plugin for Emulating the Sound Processing Unit of the Playstation 2
 * Developed and maintained by the Pcsx2 Development Team.
 *
 * Original portions from SPU2ghz are (c) 2008 by David Quintana [gigaherz]
 *
 * SPU2-X is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Found-
 * ation, either version 3 of the License, or (at your option) any later version.
 *
 * SPU2-X is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with SPU2-X.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Global.h"

#define _WIN32_DCOM
#include "Dialogs.h"

#include "portaudio.h"

#include "wchar.h"

#include <vector>

#ifdef __WIN32__
#include "pa_win_wasapi.h"

static BOOL CALLBACK _ConfigProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
#endif

int PaCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData );

//////////////////////////////////////////////////////////////////////////////////////////
// Stuff necessary for speaker expansion
class SampleReader
{
public:
	virtual int ReadSamples( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData ) = 0;
};

template<class T>
class ConvertedSampleReader : public SampleReader
{
	int* written;
public:
	ConvertedSampleReader(int* pWritten)
	{
		written = pWritten;
	}

	virtual int ReadSamples( const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData )
	{
		T* p1 = (T*)outputBuffer;

		int packets = framesPerBuffer / SndOutPacketSize;

		for(int p=0; p<packets; p++, p1+=SndOutPacketSize )
			SndBuffer::ReadSamples( p1 );

		(*written) += packets * SndOutPacketSize;

		return 0;
	}
};

Portaudio::Portaudio()
{
	m_ApiId=-1;
	m_SuggestedLatencyMinimal = true;
	m_SuggestedLatencyMS = 20;

	actualUsedChannels = 0;
}

s32 Portaudio::Init()
{
	started=false;
	stream=NULL;

	ReadSettings();

	PaError err = Pa_Initialize();
	if( err != paNoError )
	{
		fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );
		return -1;
	}
	started=true;

	int deviceIndex = -1;

	fprintf(stderr,"* SPU2-X: Enumerating PortAudio devices:\n");
	for(int i=0, j=0;i<Pa_GetDeviceCount();i++)
	{
		const PaDeviceInfo * info = Pa_GetDeviceInfo(i);

		if(info->maxOutputChannels > 0)
		{
			const PaHostApiInfo * apiinfo = Pa_GetHostApiInfo(info->hostApi);

			fprintf(stderr," *** Device %d: '%s' (%s)", j, info->name, apiinfo->name);

			if(apiinfo->type == m_ApiId)
			{
				if(m_Device == wxString::FromAscii(info->name))
				{
					deviceIndex = i;
					fprintf(stderr," (selected)");
				}

			}
			fprintf(stderr,"\n");

			j++;
		}
	}
	fflush(stderr);

	if(deviceIndex<0 && m_ApiId>=0)
	{
		for(int i=0;i<Pa_GetHostApiCount();i++)
		{
			const PaHostApiInfo * apiinfo = Pa_GetHostApiInfo(i);
			if(apiinfo->type == m_ApiId)
			{
				deviceIndex = apiinfo->defaultOutputDevice;
			}
		}
	}

	if(deviceIndex>=0)
	{
		void* infoPtr = NULL;
			
		const PaDeviceInfo * devinfo = Pa_GetDeviceInfo(deviceIndex);
		const PaHostApiInfo * apiinfo = Pa_GetHostApiInfo(devinfo->hostApi);
			
		int speakers;		
		switch(numSpeakers) // speakers = (numSpeakers + 1) *2; ?
		{
			case 0: speakers = 2; break; // Stereo
			case 1: speakers = 4; break; // Quadrafonic
			case 2: speakers = 6; break; // Surround 5.1
			case 3: speakers = 8; break; // Surround 7.1
			default: speakers = 2;
		}
		actualUsedChannels = std::min(speakers, devinfo->maxOutputChannels);

		switch( actualUsedChannels )
		{
			case 2:
				ConLog( "* SPU2 > Using normal 2 speaker stereo output.\n" );
				SampleReader = new ConvertedSampleReader<Stereo20Out32>(&writtenSoFar);
			break;

			case 3:
				ConLog( "* SPU2 > 2.1 speaker expansion enabled.\n" );
				SampleReader = new ConvertedSampleReader<Stereo21Out32>(&writtenSoFar);
			break;

			case 4:
				ConLog( "* SPU2 > 4 speaker expansion enabled [quadraphenia]\n" );
				SampleReader = new ConvertedSampleReader<Stereo40Out32>(&writtenSoFar);
			break;

			case 5:
				ConLog( "* SPU2 > 4.1 speaker expansion enabled.\n" );
				SampleReader = new ConvertedSampleReader<Stereo41Out32>(&writtenSoFar);
			break;

			case 6:
			case 7:
				switch(dplLevel)
				{
				case 0:
					ConLog( "* SPU2 > 5.1 speaker expansion enabled.\n" );
					SampleReader = new ConvertedSampleReader<Stereo51Out32>(&writtenSoFar);   //"normal" stereo upmix
					break;
				case 1:
					ConLog( "* SPU2 > 5.1 speaker expansion with basic ProLogic dematrixing enabled.\n" );
					SampleReader = new ConvertedSampleReader<Stereo51Out32Dpl>(&writtenSoFar); // basic Dpl decoder without rear stereo balancing
					break;
				case 2:
					ConLog( "* SPU2 > 5.1 speaker expansion with experimental ProLogicII dematrixing enabled.\n" );
					SampleReader = new ConvertedSampleReader<Stereo51Out32DplII>(&writtenSoFar); //gigas PLII
					break;
				}
				actualUsedChannels = 6; // we do not support 7.0 or 6.2 configurations, downgrade to 5.1
			break;

			default:	// anything 8 or more gets the 7.1 treatment!
				ConLog( "* SPU2 > 7.1 speaker expansion enabled.\n" );
				SampleReader = new ConvertedSampleReader<Stereo71Out32>(&writtenSoFar);
				actualUsedChannels = 8; // we do not support 7.2 or more, downgrade to 7.1
			break;
		}

#ifdef __WIN32__
		PaWasapiStreamInfo info = {
			sizeof(PaWasapiStreamInfo),
			paWASAPI,
			1,
			paWinWasapiExclusive
		};

		if((m_ApiId == paWASAPI) && m_WasapiExclusiveMode)
		{
			// Pass it the Exclusive mode enable flag
			infoPtr = &info;
		}
#endif

		PaStreamParameters outParams = {
			deviceIndex,
			actualUsedChannels,
			paInt32,
			m_SuggestedLatencyMinimal ?
						    (SndOutPacketSize/(float)SampleRate)
						  : (m_SuggestedLatencyMS/1000.0f),
			infoPtr
		};

		err = Pa_OpenStream(&stream,
			NULL, &outParams, SampleRate,
			SndOutPacketSize,
			paNoFlag,
			PaCallback,

			NULL);
	}
	else
	{
		err = Pa_OpenDefaultStream( &stream,
			0, actualUsedChannels, paInt32, 48000,
			SndOutPacketSize,
			PaCallback,
			NULL );
	}
	if( err != paNoError )
	{
		fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );
		Pa_Terminate();
		return -1;
	}

	err = Pa_StartStream( stream );
	if( err != paNoError )
	{
		fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );
		Pa_CloseStream(stream);
		stream=NULL;
		Pa_Terminate();
		return -1;
	}

	return 0;
}

void Portaudio::Close()
{
	PaError err;
	if(started)
	{
		if(stream)
		{
			if(Pa_IsStreamActive(stream))
			{
				err = Pa_StopStream(stream);
				if( err != paNoError )
					fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );
			}

			err = Pa_CloseStream(stream);
			if( err != paNoError )
				fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );

			stream=NULL;
		}

		// Seems to do more harm than good.
		//PaError err = Pa_Terminate();
		//if( err != paNoError )
		//	fprintf(stderr,"* SPU2-X: PortAudio error: %s\n", Pa_GetErrorText( err ) );

		started=false;
	}
}
	
int Portaudio::GetEmptySampleCount()
{
	long availableNow = Pa_GetStreamWriteAvailable(stream);

	int playedSinceLastTime = (writtenSoFar - writtenLastTime) + (availableNow - availableLastTime);
	writtenLastTime = writtenSoFar;
	availableLastTime = availableNow;

	// Lowest resolution here is the SndOutPacketSize we use.
	return playedSinceLastTime;
}
	
void Portaudio::ReadSettings()
{
	wxString api( L"EMPTYEMPTYEMPTY" );
	m_Device = L"EMPTYEMPTYEMPTY";
#ifdef __LINUX__
	// By default on linux use the ALSA API (+99% users) -- Gregory
	CfgReadStr( L"PORTAUDIO", L"HostApi", api, L"ALSA" );
#else
	CfgReadStr( L"PORTAUDIO", L"HostApi", api, L"Unknown" );
#endif
	CfgReadStr( L"PORTAUDIO", L"Device", m_Device, L"default" );

	SetApiSettings(api);

	m_WasapiExclusiveMode = CfgReadBool( L"PORTAUDIO", L"Wasapi_Exclusive_Mode", false);
	m_SuggestedLatencyMinimal = CfgReadBool( L"PORTAUDIO", L"Minimal_Suggested_Latency", true);
	m_SuggestedLatencyMS = CfgReadInt( L"PORTAUDIO", L"Manual_Suggested_Latency_MS", 20);
		
	if( m_SuggestedLatencyMS < 10 ) m_SuggestedLatencyMS = 10;
	if( m_SuggestedLatencyMS > 200 ) m_SuggestedLatencyMS = 200;
}

void Portaudio::SetApiSettings(wxString api)
{
	m_ApiId = -1;
	if(api == L"InDevelopment") m_ApiId = paInDevelopment; /* use while developing support for a new host API */
	if(api == L"DirectSound")	m_ApiId = paDirectSound;
	if(api == L"MME")			m_ApiId = paMME;
	if(api == L"ASIO")			m_ApiId = paASIO;
	if(api == L"SoundManager")	m_ApiId = paSoundManager;
	if(api == L"CoreAudio")		m_ApiId = paCoreAudio;
	if(api == L"OSS")			m_ApiId = paOSS;
	if(api == L"ALSA")			m_ApiId = paALSA;
	if(api == L"AL")			m_ApiId = paAL;
	if(api == L"BeOS")			m_ApiId = paBeOS;
	if(api == L"WDMKS")			m_ApiId = paWDMKS;
	if(api == L"JACK")			m_ApiId = paJACK;
	if(api == L"WASAPI")		m_ApiId = paWASAPI;
	if(api == L"AudioScienceHPI") m_ApiId = paAudioScienceHPI;
}

void Portaudio::WriteSettings() const
{
	wxString api;
	switch(m_ApiId)
	{
	case paInDevelopment:	api = L"InDevelopment"; break; /* use while developing support for a new host API */
	case paDirectSound:		api = L"DirectSound"; break;
	case paMME:				api = L"MME"; break;
	case paASIO:			api = L"ASIO"; break;
	case paSoundManager:	api = L"SoundManager"; break;
	case paCoreAudio:		api = L"CoreAudio"; break;
	case paOSS:				api = L"OSS"; break;
	case paALSA:			api = L"ALSA"; break;
	case paAL:				api = L"AL"; break;
	case paBeOS:			api = L"BeOS"; break;
	case paWDMKS:			api = L"WDMKS"; break;
	case paJACK:			api = L"JACK"; break;
	case paWASAPI:			api = L"WASAPI"; break;
	case paAudioScienceHPI: api = L"AudioScienceHPI"; break;
	default: api = L"Unknown";
	}

	CfgWriteStr( L"PORTAUDIO", L"HostApi", api);
	CfgWriteStr( L"PORTAUDIO", L"Device", m_Device);

	CfgWriteBool( L"PORTAUDIO", L"Wasapi_Exclusive_Mode", m_WasapiExclusiveMode);
	CfgWriteBool( L"PORTAUDIO", L"Minimal_Suggested_Latency", m_SuggestedLatencyMinimal);
	CfgWriteInt( L"PORTAUDIO", L"Manual_Suggested_Latency_MS", m_SuggestedLatencyMS);
}

static Portaudio PA;
Portaudio *SndOut = &PA;

void SndOutReassign()
{
	SndOut = &PA;
}

int PaCallback( const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData )
{
	return ((SampleReader*)PA.SampleReader)->ReadSamples(inputBuffer,outputBuffer,framesPerBuffer,timeInfo,statusFlags,userData);
}
