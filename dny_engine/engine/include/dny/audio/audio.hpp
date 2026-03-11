#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

namespace dny{
	enum class audio_state : std::uint8_t{
		stopped,
		playing,
		paused
	};

	class audio_clip{
	public:
		audio_clip();
		~audio_clip();

		audio_clip( audio_clip const& ) = delete;
		audio_clip& operator=( audio_clip const& ) = delete;

		audio_clip( audio_clip&& ) noexcept;
		audio_clip& operator=( audio_clip&& ) noexcept;

		void play();
		void stop();
		void pause();
		void resume();

		void set_loop( bool loop );
		void set_volume( float volume );

		bool is_playing() const;
		bool is_looping() const;
		audio_state get_state() const;

	private:
		friend class audio_engine;
		struct impl;
		std::unique_ptr<impl> m_impl;

		explicit audio_clip( std::unique_ptr<impl>&& impl );
	};

	class audio_engine{
	public:
		audio_engine();
		~audio_engine();

		audio_engine( audio_engine const& ) = delete;
		audio_engine& operator=( audio_engine const& ) = delete;

		audio_engine( audio_engine&& ) = delete;
		audio_engine& operator=( audio_engine&& ) = delete;

		audio_clip load_audio( std::filesystem::path const& filename );

		void set_master_volume( float volume );
		float get_master_volume() const;

		void update();

	private:
		struct impl;
		std::unique_ptr<impl> m_impl;
	};
}
