#pragma once

#include <chrono>

namespace dny{
	class Timer{
	public:
		float mark(){
			auto end = clock_type::now();
			auto elapsed = std::chrono::duration<float>{ end - m_start }.count();
			m_start = end;
			return elapsed;
		}
	private:
		using clock_type = std::chrono::high_resolution_clock;
		using time_point = clock_type::time_point;
		time_point m_start = clock_type::now();
	};


}