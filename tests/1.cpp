#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"

class MyApp : public wil::App
{
public:
	void OnInit(wil::AppInitCtx &ctx) override {
		ctx.window.size = {1600, 900};
	}
};

int main(int argc, char **argv) {
	MyApp app;
	wil::appimpl(&app, argc, argv);
	return 0;
}
