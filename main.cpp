#include "range/v3/algorithm/max.hpp"
#include "range/v3/distance.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/to_container.hpp"
#include "range/v3/view/all.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/join.hpp"
#include "range/v3/view/partial_sum.hpp"
#include "range/v3/view/transform.hpp"
#include "range/v3/view/zip.hpp"

#include "boost/coroutine2/all.hpp"

#include <iostream>
#include <iterator>
#include <set>
#include <vector>

using namespace ranges;

namespace
{

using FlutCost = int;
using Fluts    = std::vector<FlutCost>;
using Schuurs  = std::vector<Fluts>;

constexpr FlutCost sale_price = 10;

Schuurs parse_schuurs( std::istream& input )
{
  size_t schuurs_num = 0;
  input >> schuurs_num;

  Schuurs schuurs;
  while ( schuurs_num-- )
  {
    size_t fluts_num = 0;
    input >> fluts_num;

    Fluts fluts;
    fluts.reserve( fluts_num );

    while ( fluts_num-- )
    {
      FlutCost flut;
      input >> flut;
      fluts.push_back( flut );
    }

    schuurs.push_back( std::move( fluts ) );
  }

  return schuurs;
}

using FlutIndices = std::vector<int>;
using Coro        = boost::coroutines2::coroutine<int>;

template <typename Iter, typename End>
void find_numbers_of_flut( Iter schuurs_iter, End schuurs_end, Coro::push_type& yield, FlutIndices& indices )
{
  if ( schuurs_iter == schuurs_end )
  {
    yield( accumulate( indices, 0 ) );
    return;
  }

  auto [ partial_sums, max_sum ] = *schuurs_iter;
  for ( auto [ sum, index ] : view::zip( partial_sums, view::iota( 1 ) ) )
  {
    if ( sum == max_sum )
    {
      indices.push_back( index );
      find_numbers_of_flut( schuurs_iter + 1, schuurs_end, yield, indices );
      indices.pop_back();
    }
  }
}

}  // namespace

int main()
{
  const auto schuurs = parse_schuurs( std::cin );
  if( schuurs.empty() )
  {
      return 0;
  }

  const auto schuurs_partial_sums =
      schuurs | view::transform( []( const auto& schuur ) {
        return schuur | view::transform( []( const auto cost ) { return sale_price - cost; } ) | view::partial_sum;
      } );

  const auto max_sums = schuurs_partial_sums | view::transform( []( const auto schuur_partial_sum ) { return max( schuur_partial_sum ); } );

  Coro::pull_type numbers_of_flut_coro( [&schuurs_partial_sums, &max_sums]( Coro::push_type& yield ) {
    auto indexed_sums = view::zip( schuurs_partial_sums, max_sums );

    FlutIndices indices;
    find_numbers_of_flut( indexed_sums.begin(), indexed_sums.end(), yield, indices );
  } );

  const FlutCost max_profit           = accumulate( max_sums, FlutCost{ 0 } );
  const std::set<int> numbers_of_flut = numbers_of_flut_coro | view::all;

  std::cout << "schuurs " << schuurs.size() << '\n';
  std::cout << "Maximum profit is " << max_profit << '\n';
  std::cout << "Number of fluts to buy ";
  std::copy( numbers_of_flut.begin(), numbers_of_flut.end(), std::ostream_iterator<int>( std::cout, " " ) );
  std::cout << std::endl;

  return main();
}
