#pragma once

#include "dny_math.hpp"
#include "dny_physics.hpp"

#include <array>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

namespace dny{
	template<typename Object>
	class qtree{
	public:
		using object_type = Object;
		using iterator = typename std::vector<object_type>::iterator;
		using const_iterator = typename std::vector<object_type>::const_iterator;
	public:
		class data{
		public:
			data( Rect<float> const& bounds, object_type* pObj )noexcept
				:
				m_bounds( bounds ),
				m_pObject( pObj ){}
			Rect<float> const& bounds()const noexcept{
				return m_bounds;
			}
			object_type const& object()const noexcept{
				return *m_pObject;
			}
			object_type& object()noexcept{
				return *m_pObject;
			}
		private:
			Rect<float> m_bounds = {};
			object_type* m_pObject = nullptr;
		};

		class node;

		qtree(
			Rect<float> const& bounds_,
			std::int32_t max_objects_per_node_,
			float min_node_width_ )
			:
			m_max_objects_per_node( max_objects_per_node_ ),
			m_min_node_width( min_node_width_ ){
			m_nodes.emplace_back( *this, bounds_, m_nodes.size() );
		}


		void add_object( object_type& object_, Rect<float> const& aabb_ ){
			m_nodes.front().add_object( data{ aabb_, &object_ }, *this );
		}

		template<typename Action>
		void generate_pairs( Action&& action ){
			if( !m_nodes.empty() )
				m_nodes.front().generate_pairs( action );
		}

		void clear()noexcept{
			const auto root_bounds = m_nodes.front().bounds();
			m_nodes.clear();
			m_nodes.emplace_back( *this, root_bounds, m_nodes.size() );
			m_max_depth = {};
		}

		template<typename Action>
		void query( Rect<float> const& bounds_, Action&& action_ ){
			m_nodes.front().query( bounds_, action_ );
		}

		// Diagnostics
		std::size_t max_depth()const noexcept{
			return m_max_depth;
		}
	private:
		std::vector<node> m_nodes;
		std::int32_t m_max_objects_per_node = 100;
		float m_min_node_width = {};
		std::size_t m_depth = {};
		std::size_t m_max_depth = {};
	};

	template<typename Object>
	class qtree<Object>::node{
	public:
		using base_ = qtree<Object>;

	public:
		node( base_& tree_, Rect<float> const& bounds_, std::size_t id_ )
			:
			m_node_bounds( bounds_ ),
			m_tree( &tree_ ),
			m_id( id_ ){
			m_data.reserve( tree_.m_max_objects_per_node );
		}

		std::vector<base_::data>& elements()noexcept{
			return m_data;
		}
		std::vector<base_::data> const& elements()const noexcept{
			return m_data;
		}
		dny::Rect<float> const& bounds()const noexcept{
			return m_node_bounds;
		}

	private:
		template<typename Action>
		void generate_pairs( Action&& action ){
			// 1) Test pairs within this node
			const auto count = m_data.size();
			for( std::size_t i = 0; i < count; ++i ){
				for( std::size_t j = i + 1; j < count; ++j ){
					action( m_data[ i ].object(), m_data[ j ].object() );
				}
			}

			// 2) Test parent objects against children
			for( auto child_id : m_child_indices ){
				if( child_id == not_assigned )
					continue;

				auto& child = m_tree->m_nodes[ child_id ];

				// parent ↔ child cross test
				for( auto& parent_obj : m_data ){
					for( auto& child_obj : child.m_data ){
						action( parent_obj.object(), child_obj.object() );
					}
				}

				// recurse
				child.generate_pairs( action );
			}
		}
		template<typename Action>
		void query( Rect<float> const& query_bounds_, Action&& action_ ){
			if( intersects( m_node_bounds, query_bounds_ ) ){
				for( auto& element : m_data ){
					if( intersects( element.bounds(), query_bounds_ ) )
						if( !action_( element.object() ) ){
							return;
						}
				}
				for( auto child_id : m_child_indices ){
					if( child_id == not_assigned )continue;

					auto& child = m_tree->m_nodes[ child_id ];
					child.query( query_bounds_, action_ );
				}
			}
		}
		Rect<float> get_quadrant( int index )const noexcept{
			const auto center = m_node_bounds.center();

			switch( index ){
				case 0: // left top
					return dny::Rect<float>( m_node_bounds.left, m_node_bounds.top, center.x, center.y );
				case 1: // right top													         
					return dny::Rect<float>( center.x, m_node_bounds.top, m_node_bounds.right, center.y );
				case 2: // left bottom 
					return dny::Rect<float>( m_node_bounds.left, center.y, center.x, m_node_bounds.bottom );
				case 3: // right bottom
					return dny::Rect<float>( center.x, center.y, m_node_bounds.right, m_node_bounds.bottom );
			}

			// TODO: assert( index >= 0 && index < 4 && "Index out of range");
			return dny::Rect<float>{};
		}

		void add_to_child( data data_, base_& tree_ ){
			++tree_.m_depth;
			tree_.m_max_depth = std::max( tree_.m_depth, tree_.m_max_depth );

			for( int index = 0; index < 4; ++index ){
				const auto quadrant = get_quadrant( index );
				if( quadrant.width() < tree_.m_min_node_width ){
					m_data.push_back( data_ );
					break;
				}

				if( !intersects( quadrant, data_.bounds() ) )
					continue;

				auto child_index = m_child_indices[ index ];
				if( child_index == not_assigned ){
					child_index = tree_.m_nodes.size();
					m_child_indices[ index ] = child_index;
					tree_.m_nodes.emplace_back( tree_, quadrant, child_index );
				}

				auto& child = tree_.m_nodes[ child_index ];

				child.add_object( data_, tree_ );
				break;
			}

			--tree_.m_depth;
		}
		void add_object( data const& data_, base_& tree_ ){
			if( m_data.size() < m_tree->m_max_objects_per_node ){
				m_data.push_back( data_ );
			}
			else{
				add_to_child( data_, tree_ );
			}
		}
	private:
		friend class base_;
		static constexpr std::size_t not_assigned = -1;

		std::array<std::size_t, 4> m_child_indices = { not_assigned, not_assigned, not_assigned, not_assigned };
		std::vector<base_::data> m_data;
		Rect<float> m_node_bounds;
		base_* m_tree = nullptr;
		std::size_t m_id = not_assigned;
	};
}
