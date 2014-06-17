#ifndef INCLUDED_scheme_nest_NEST_test_HH
#define INCLUDED_scheme_nest_NEST_test_HH

#include <nest/NEST.hh>
#include <gtest/gtest.h>
#include <boost/assign/std/vector.hpp> // for 'operator+=()'
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>

namespace scheme {
namespace nest {

using std::cout;
using std::endl;

TEST(NEST,particular_cases){
	NEST<1>	nest;
	ASSERT_EQ( nest.set_and_get(0,0)[0], 0.5 );
	ASSERT_EQ( nest.set_and_get(0,1)[0], 0.25 );
	ASSERT_EQ( nest.set_and_get(1,1)[0], 0.75 );
	ASSERT_EQ( nest.set_and_get(0,2)[0], 0.125 );
	ASSERT_EQ( nest.set_and_get(1,2)[0], 0.375 );
	ASSERT_EQ( nest.set_and_get(2,2)[0], 0.625 );
	ASSERT_EQ( nest.set_and_get(3,2)[0], 0.875 );
	NEST<2>	nest2;
	ASSERT_EQ( nest2.set_and_get(0,0), Vector2f(0.5,0.5)   );
	ASSERT_EQ( nest2.set_and_get(0,1), Vector2f(0.25,0.25)   );
	ASSERT_EQ( nest2.set_and_get(1,1), Vector2f(0.75,0.25)   );
	ASSERT_EQ( nest2.set_and_get(2,1), Vector2f(0.25,0.75)   );
	ASSERT_EQ( nest2.set_and_get(3,1), Vector2f(0.75,0.75)   );
}

TEST(NEST,map_discrete) {
	using std::cout;
	using std::endl;

	using namespace boost::assign;
	std::vector<double> choices;
	choices += 42.0,152.345,8049782.83402;
	NEST<0,double,DiscreteChoiceMap,StoreValue> nest0(choices);
	ASSERT_EQ(choices.size(),nest0.size());
	ASSERT_EQ(choices.size(),nest0.size(0));	
	ASSERT_EQ(choices.size(),nest0.size(3));
	ASSERT_EQ(choices.size(),nest0.size(4));			
	for(size_t i = 0; i < nest0.size(); ++i){
		ASSERT_TRUE( nest0.set_state(i) );
		ASSERT_EQ(choices[i],nest0.set_and_get(i));
		ASSERT_EQ(choices[i],nest0.set_and_get(i,0));
		ASSERT_EQ(choices[i],nest0.set_and_get(i,7));
		ASSERT_EQ(choices[i],nest0.set_and_get(i,3));
		ASSERT_EQ(choices[i],nest0.set_and_get(i,8));
	}
	ASSERT_FALSE( nest0.set_state(5) );
}


template<int DIM>
void test_coverage_cube(){
	boost::random::mt19937 rng; 
	boost::uniform_real<> uniform;
	NEST<DIM> nest;
	size_t rmax = 9/DIM+1;
	for(size_t r = 0; r <= rmax; ++r){
		float cellradius = sqrt(DIM) * 0.5 / (1<<r);
		for(size_t iter = 0; iter < 10000/DIM; ++iter){
			Matrix<float,DIM,1> tgt;
			for(size_t i = 0; i < DIM; ++i) tgt[i] = uniform(rng);
			size_t index = nest.get_index(tgt,r);
			Matrix<float,DIM,1> val = nest.set_and_get(index,r);
			ASSERT_LE( (tgt-val).norm(), cellradius );
		}
		Matrix<float,DIM,1> zeros; zeros.fill(0);
		ASSERT_LE( (zeros-nest.set_and_get(nest.get_index(zeros,r),r)).norm(), cellradius );
		Matrix<float,DIM,1> ones; ones.fill(0.999999);
		ASSERT_LE( (ones-nest.set_and_get(nest.get_index(ones,r),r)).norm(), cellradius );
	}
}

TEST(NEST,coverage_cube){
	test_coverage_cube<1>();
	test_coverage_cube<2>();
	test_coverage_cube<3>();
	test_coverage_cube<4>();
	test_coverage_cube<5>();
	test_coverage_cube<6>();
}


template<int DIM>
void test_index_nesting(){
	typedef Matrix<float,DIM,1> VAL;
	NEST<DIM> nest(2);
	size_t rmax = 9/DIM+1;
	for(size_t r = 0; r <= rmax; ++r){
		for(size_t i = 0; i < nest.size(r); ++i){
			ASSERT_TRUE( nest.set_state(i,r) );
			VAL value = nest.value();
			size_t index = nest.get_index(value,r);
			ASSERT_EQ( i, index );
			for(size_t r2 = 0; r2 <= r; ++r2){
				size_t index2 = nest.get_index(value,r2);
				ASSERT_EQ( i>>(DIM*(r-r2)), index2 );
			}
		}
	}
}
TEST(NEST,index_nesting){
	test_index_nesting<1>();
	test_index_nesting<2>();
	test_index_nesting<3>();
	test_index_nesting<4>();
	test_index_nesting<5>();
	test_index_nesting<6>();
}

template<int DIM>
void test_store_pointer(){
	typedef Matrix<float,DIM,1> VAL;
	NEST<DIM,VAL,UnitMap,StoreValue  > nest_val(2);
	NEST<DIM,VAL,UnitMap,StorePointer> nest_ptr(2);
	VAL val;
	nest_ptr.set_pointer(&val);
	size_t rmax = 9/DIM+1;
	for(size_t r = 0; r <= rmax; ++r){
		for(size_t i = 0; i < nest_val.size(r); ++i){
			ASSERT_TRUE( nest_val.set_state(i,r) );
			ASSERT_TRUE( nest_ptr.set_state(i,r) );			
			ASSERT_EQ( nest_val.value(), val );
		}
	}
}
TEST(NEST,store_pointer){
	test_store_pointer<1>();
	test_store_pointer<2>();
	test_store_pointer<3>();
	test_store_pointer<4>();
	test_store_pointer<5>();
	test_store_pointer<6>();
}

template<int DIM>
void test_uniformity(){
	typedef Matrix<float,DIM,1> VAL;
	NEST<DIM,VAL,UnitMap,StoreValue  > nest(1);
	size_t rmax = 9/DIM+1;
	for(size_t r = 0; r <= rmax; ++r){
		float scale = 1<<r;
		ArrayXi counts(1<<r); counts.fill(0);
		for(size_t i = 0; i < nest.size(r); ++i){
			ASSERT_TRUE( nest.set_state(i,r) );
			for(size_t j = 0; j < DIM; ++j){
				counts[nest.value()[j]*scale]++;
			}
		}
		ASSERT_EQ( (1<<((DIM-1)*r))*DIM, counts.minCoeff() );		
		ASSERT_EQ( (1<<((DIM-1)*r))*DIM, counts.maxCoeff() );				
	}
}
TEST(NEST,uniformity){
	test_uniformity<1>();
	test_uniformity<2>();
	test_uniformity<3>();
	test_uniformity<4>();
	test_uniformity<5>();
	test_uniformity<6>();
}

template<int DIM>
void test_index_lookup_scaled(){
	typedef Matrix<float,DIM,1> VAL;
	Matrix<float,10,1> lb0,ub0;
	Matrix<size_t,10,1> bs0;
	lb0 << -1.3,2.2,0,-3,-5,-9.9,1.3,44,-13.3,99;
	ub0 <<  1.3,4.2,1,10,-3, 9.9,4.3,44, 13.3,199;
	bs0 << 2,1,2,3,4,5,6,7,8,9;

	typedef Matrix<float,DIM,1> VAL;
	VAL lb, ub;
	Matrix<size_t,DIM,1> bs;
	for(size_t i = 0; i < DIM; ++i){
		lb[i] = lb0[i]; ub[i] = ub0[i]; bs[i] = bs0[i];
	}
	NEST<DIM,VAL,ScaleMap,StoreValue> nest(lb,ub,bs);
	size_t rmax = 6/DIM;
	for(size_t r = 0; r <= rmax; ++r){
		for(size_t i = 0; i < nest.size(r); ++i){
			ASSERT_TRUE( nest.set_state(i,r) );
			VAL value = nest.value();
			size_t index = nest.get_index(value,r);
			ASSERT_EQ( i, index );
			for(size_t r2 = 0; r2 <= r; ++r2){
				size_t index2 = nest.get_index(value,r2);
				ASSERT_EQ( i>>(DIM*(r-r2)), index2 );
			}
		}
	}
}
TEST(NEST,index_lookup_scaled){
	test_index_lookup_scaled<1>();
	test_index_lookup_scaled<2>();
	test_index_lookup_scaled<3>();
	test_index_lookup_scaled<4>();
	test_index_lookup_scaled<5>();
	test_index_lookup_scaled<6>();
}


template<int DIM>
void test_map_scale_bounds(){
	BOOST_STATIC_ASSERT(DIM<10);
	Matrix<float,10,1> lb0,ub0;
	Matrix<size_t,10,1> bs0;
	lb0 << -1.3,2.2,0,-3,-5,-9.9,1.3,44,-13.3,99;
	ub0 <<  1.3,4.2,1,10,-3, 9.9,4.3,44, 13.3,199;
	bs0 << 1,2,3,4,5,6,7,8,9,10;

	typedef Matrix<float,DIM,1> VAL;
	VAL lb, ub;
	Matrix<size_t,DIM,1> bs;
	for(size_t i = 0; i < DIM; ++i){
		lb[i] = lb0[i]; ub[i] = ub0[i]; bs[i] = bs0[i];
	}
	NEST<DIM,VAL,ScaleMap,StoreValue> nest(lb,ub,bs);

	size_t resl = 6/DIM+1;
	for(size_t i = 0; i < nest.size(resl); ++i){
		ASSERT_TRUE( nest.set_state(i,resl) );
		for(size_t j = 0; j < DIM; ++j){ 
			ASSERT_LT( lb[j], nest.value()[j] ); ASSERT_LT( nest.value()[j] , ub[j] );
		}
		ASSERT_FALSE( nest.set_state(i+nest.size(resl),resl) );
		for(size_t j = 0; j < DIM; ++j){ 
			ASSERT_LT( lb[j], nest.value()[j] ); ASSERT_LT( nest.value()[j] , ub[j] );
		}
	}

}
TEST(NEST,map_scale){
	test_map_scale_bounds<1>();
	test_map_scale_bounds<2>();
	test_map_scale_bounds<3>();
	test_map_scale_bounds<4>();
	// test_map_scale_bounds<5>();
	// test_map_scale_bounds<6>();
}



}
}

#endif