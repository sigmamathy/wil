#include <wil/algebra.hpp>
#include <wil/log.hpp>
#include <wil/ecs.hpp>

using namespace wil;

struct Test {
	float val;
};

Registry registry;

class TestSystem : public System
{
public:

	TestSystem(ComponentManager &cm) : System(cm) {}

	void process() {
		for (Entity e : GetEntities()) {
			Test test = registry.GetComponent<Test>(e);
			test.val *= 2.f;
			WIL_LOGINFO("{}", test.val);
		}
	}

	Signature GetSignature() const override {
		return MakeSignature<Test>();
	}
};

int main()
{
	registry.RegisterComponent<Test>();
	auto &s = registry.RegisterSystem<TestSystem>();

	Entity e = registry.CreateEntity();
	registry.AddComponent(e, Test{1.0f});

	s.process();

	registry.RemoveComponent<Test>(e);

	s.process();
}
