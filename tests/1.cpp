#include "wil/appimpl.hpp"
#include "wil/algebra.hpp"

class MyLayer : public wil::Layer3D
{
public:
	std::string GetName() const override { return "MyLayer"; }
};

class MyApp : public wil::App
{
public:
	void OnInit(wil::AppInitCtx &ctx) override {
		ctx.window.size = {1600, 900};
		ctx.window.title = "MyApp";

		ctx.NewLayer<MyLayer>();
	}
};

int main(int argc, char **argv) {
	MyApp app;
	wil::appimpl(&app, argc, argv);
	return 0;
}
