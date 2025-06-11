#include <wil/algebra.hpp>
#include <wil/log.hpp>
#include <wil/ecs.hpp>

using namespace wil;

struct Test {
	float val;
};

struct Test2 {
	Ivec2 wow;
};


class TestSystem : public System
{
public:

	EntityView view;

	TestSystem(Registry &registry) : System(registry) {
		registry.RegisterEntityView<Test, Test2>(view);
	}

	void process(Registry &registry)
	{
		for (Entity e : view.set)
		{
			auto [t, t2] = registry.GetComponents<Test, Test2>(e);
			t.val *= 2.f;
			WIL_LOGINFO("{}", t.val + t2.wow.Norm());
		}
	}
};

int main()
{
	Registry registry;
	auto &s = registry.RegisterSystem<TestSystem>();

	Entity e = registry.CreateEntity();
	registry.AddComponents(e, Test{1.0f}, Test2{Ivec2{4, 3}});

	s.process(registry);
	s.process(registry);

	registry.RemoveComponents<Test>(e);

	s.process(registry);
}
