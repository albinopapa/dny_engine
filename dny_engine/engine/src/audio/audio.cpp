#include "audio/audio.hpp"

#include "platform/win32sdk.hpp"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <xaudio2.h>
#include <wrl/client.h>

#include <algorithm>
#include <vector>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "xaudio2.lib")

using Microsoft::WRL::ComPtr;

namespace dny{
	struct audio_buffer{
		std::vector<std::uint8_t> data;
		WAVEFORMATEX format = {};
	};

	audio_buffer load_audio_data( std::filesystem::path const& filename ){
		CoInitializeEx( nullptr, COINIT_MULTITHREADED );
		MFStartup( MF_VERSION );

		ComPtr<IMFSourceReader> reader;
		auto hr = MFCreateSourceReaderFromURL(
			filename.wstring().c_str(),
			nullptr,
			&reader
		);

		if( FAILED( hr ) ){
			MFShutdown();
			return {};
		}

		ComPtr<IMFMediaType> media_type;
		MFCreateMediaType( &media_type );
		media_type->SetGUID( MF_MT_MAJOR_TYPE, MFMediaType_Audio );
		media_type->SetGUID( MF_MT_SUBTYPE, MFAudioFormat_PCM );
		reader->SetCurrentMediaType(
			static_cast<DWORD>( MF_SOURCE_READER_FIRST_AUDIO_STREAM ),
			nullptr,
			media_type.Get()
		);

		ComPtr<IMFMediaType> output_type;
		reader->GetCurrentMediaType(
			static_cast<DWORD>( MF_SOURCE_READER_FIRST_AUDIO_STREAM ),
			&output_type
		);

		WAVEFORMATEX* wave_format = nullptr;
		UINT32 format_size = 0;
		MFCreateWaveFormatExFromMFMediaType(
			output_type.Get(),
			&wave_format,
			&format_size
		);

		audio_buffer buffer;
		buffer.format = *wave_format;
		CoTaskMemFree( wave_format );

		while( true ){
			ComPtr<IMFSample> sample;
			DWORD flags = 0;

			hr = reader->ReadSample(
				static_cast<DWORD>( MF_SOURCE_READER_FIRST_AUDIO_STREAM ),
				0,
				nullptr,
				&flags,
				nullptr,
				&sample
			);

			if( flags & MF_SOURCE_READERF_ENDOFSTREAM ){
				break;
			}

			if( sample ){
				ComPtr<IMFMediaBuffer> media_buffer;
				sample->ConvertToContiguousBuffer( &media_buffer );

				BYTE* audio_data = nullptr;
				DWORD buffer_length = 0;
				media_buffer->Lock( &audio_data, nullptr, &buffer_length );

				buffer.data.insert(
					buffer.data.end(),
					audio_data,
					audio_data + buffer_length
				);

				media_buffer->Unlock();
			}
		}

		MFShutdown();

		return buffer;
	}

	struct audio_clip::impl{
		audio_buffer buffer;
		IXAudio2SourceVoice* source_voice = nullptr;
		IXAudio2* xaudio_engine = nullptr;
		audio_state state = audio_state::stopped;
		bool looping = false;
		float volume = 1.0f;

		~impl(){
			if( source_voice ){
				source_voice->Stop();
				source_voice->DestroyVoice();
			}
		}
	};

	struct audio_engine::impl{
		ComPtr<IXAudio2> xaudio;
		IXAudio2MasteringVoice* mastering_voice = nullptr;
		float master_volume = 1.0f;

		impl(){
			CoInitializeEx( nullptr, COINIT_MULTITHREADED );
			
			auto hr = XAudio2Create( &xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR );
			if( SUCCEEDED( hr ) ){
				xaudio->CreateMasteringVoice( &mastering_voice );
			}
		}

		~impl(){
			if( mastering_voice ){
				mastering_voice->DestroyVoice();
			}
		}
	};

	audio_clip::audio_clip() = default;
	audio_clip::~audio_clip() = default;

	audio_clip::audio_clip( audio_clip&& other ) noexcept
		: m_impl( std::move( other.m_impl ) )
	{}

	audio_clip& audio_clip::operator=( audio_clip&& other ) noexcept{
		if( this != &other ){
			m_impl = std::move( other.m_impl );
		}
		return *this;
	}

	audio_clip::audio_clip( std::unique_ptr<impl>&& impl )
		: m_impl( std::move( impl ) )
	{}

	void audio_clip::play(){
		if( !m_impl || !m_impl->source_voice ){
			return;
		}

		m_impl->source_voice->Stop();
		m_impl->source_voice->FlushSourceBuffers();

		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = static_cast<UINT32>( m_impl->buffer.data.size() );
		buffer.pAudioData = m_impl->buffer.data.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.LoopCount = m_impl->looping ? XAUDIO2_LOOP_INFINITE : 0;

		m_impl->source_voice->SubmitSourceBuffer( &buffer );
		m_impl->source_voice->Start();
		m_impl->state = audio_state::playing;
	}

	void audio_clip::stop(){
		if( !m_impl || !m_impl->source_voice ){
			return;
		}

		m_impl->source_voice->Stop();
		m_impl->source_voice->FlushSourceBuffers();
		m_impl->state = audio_state::stopped;
	}

	void audio_clip::pause(){
		if( !m_impl || !m_impl->source_voice || m_impl->state != audio_state::playing ){
			return;
		}

		m_impl->source_voice->Stop();
		m_impl->state = audio_state::paused;
	}

	void audio_clip::resume(){
		if( !m_impl || !m_impl->source_voice || m_impl->state != audio_state::paused ){
			return;
		}

		m_impl->source_voice->Start();
		m_impl->state = audio_state::playing;
	}

	void audio_clip::set_loop( bool loop ){
		if( !m_impl ){
			return;
		}

		m_impl->looping = loop;
	}

	void audio_clip::set_volume( float volume ){
		if( !m_impl || !m_impl->source_voice ){
			return;
		}

		m_impl->volume = std::clamp( volume, 0.0f, 1.0f );
		m_impl->source_voice->SetVolume( m_impl->volume );
	}

	bool audio_clip::is_playing() const{
		if( !m_impl || !m_impl->source_voice ){
			return false;
		}

		XAUDIO2_VOICE_STATE state;
		m_impl->source_voice->GetState( &state );
		return state.BuffersQueued > 0;
	}

	bool audio_clip::is_looping() const{
		return m_impl && m_impl->looping;
	}

	audio_state audio_clip::get_state() const{
		if( !m_impl ){
			return audio_state::stopped;
		}

		if( m_impl->state == audio_state::playing && !is_playing() ){
			m_impl->state = audio_state::stopped;
		}

		return m_impl->state;
	}

	audio_engine::audio_engine()
		: m_impl( std::make_unique<impl>() )
	{}

	audio_engine::~audio_engine() = default;

	audio_clip audio_engine::load_audio( std::filesystem::path const& filename ){
		auto buffer = load_audio_data( filename );
		if( buffer.data.empty() ){
			return audio_clip{ nullptr };
		}

		auto clip_impl = std::make_unique<audio_clip::impl>();
		clip_impl->buffer = std::move( buffer );
		clip_impl->xaudio_engine = m_impl->xaudio.Get();

		auto hr = m_impl->xaudio->CreateSourceVoice(
			&clip_impl->source_voice,
			&clip_impl->buffer.format
		);

		if( FAILED( hr ) ){
			return audio_clip{ nullptr };
		}

		return audio_clip{ std::move( clip_impl ) };
	}

	void audio_engine::set_master_volume( float volume ){
		if( !m_impl || !m_impl->mastering_voice ){
			return;
		}

		m_impl->master_volume = std::clamp( volume, 0.0f, 1.0f );
		m_impl->mastering_voice->SetVolume( m_impl->master_volume );
	}

	float audio_engine::get_master_volume() const{
		return m_impl ? m_impl->master_volume : 0.0f;
	}

	void audio_engine::update(){
		// This function can be used for future audio processing
		// Currently XAudio2 handles playback automatically
	}
}
