#include "gfx.h"

// global
auto gfx_backend = std::make_shared<gfx::GFX>();

void
init()
{
}

void
render()
{
	gfx_backend->setClearColor(glm::vec4(0.0f, 0.67f, 0.9f, 1.0f));
	gfx_backend->clearBuffer();
}

int
main()
{
	// initialize gfx
	// ---------------------------------------
	gfx_backend->init("gfx window", 800, 600);
	gfx_backend->on_Init(init);
	gfx_backend->on_Render(render);
	gfx_backend->start();

	return 0;
}