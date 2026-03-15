#pragma once

namespace dny
{
	// RequestT must be an 'enum class'
	template<typename RequestT>
	class IAppState{
	public:
		using request_type = RequestT;
	public:
		virtual ~IAppState() = default;

		virtual void update( float dt ) = 0;
		virtual void render()const = 0;

		request_type request()const{ return m_request; }

	protected:
		request_type m_request;
	};
}
