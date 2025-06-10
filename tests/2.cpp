#include <wil/algebra.hpp>
#include <wil/log.hpp>


int main()
{
	using namespace wil::algebra;

	Fmat2 m1 = {
		Fvec2{2, 3},
		Fvec2{5, 9}
	};

	Fmat2 m2 = {
		Fvec2{1, 5},
		Fvec2{6, 4}
	};

	Fmat2 m = m1 * m2;

	WIL_LOGINFO("{}", m1[0]);
}
