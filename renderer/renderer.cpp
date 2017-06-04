#include "renderer.hpp"
#include "device.hpp"
#include "render_context.hpp"
#include "sprite.hpp"

using namespace Vulkan;
using namespace Util;

namespace Granite
{

Renderer::Renderer()
{
	EventManager::get_global().register_latch_handler(DeviceCreatedEvent::type_id,
                                                      &Renderer::on_device_created,
                                                      &Renderer::on_device_destroyed,
                                                      this);
}

void Renderer::on_device_created(const Event &e)
{
	auto &created = e.as<DeviceCreatedEvent>();
	auto &device = created.get_device();

	suite[ecast(RenderableType::Mesh)].init_graphics(&device.get_shader_manager(), "assets://shaders/static_mesh.vert", "assets://shaders/static_mesh.frag");
	suite[ecast(RenderableType::DebugMesh)].init_graphics(&device.get_shader_manager(), "assets://shaders/debug_mesh.vert", "assets://shaders/debug_mesh.frag");
	suite[ecast(RenderableType::Skybox)].init_graphics(&device.get_shader_manager(), "assets://shaders/skybox.vert", "assets://shaders/skybox.frag");

	this->device = &device;
}

void Renderer::on_device_destroyed(const Event &)
{
}

void Renderer::begin()
{
	queue.reset();
	queue.set_shader_suites(suite);
}

void Renderer::flush(Vulkan::CommandBuffer &cmd, RenderContext &context)
{
	auto *global = static_cast<RenderParameters *>(cmd.allocate_constant_data(0, 0, sizeof(RenderParameters)));
	*global = context.get_render_parameters();

	queue.sort();

	cmd.set_opaque_state();
	CommandBufferSavedState state;
	cmd.save_state(COMMAND_BUFFER_SAVED_SCISSOR_BIT | COMMAND_BUFFER_SAVED_VIEWPORT_BIT | COMMAND_BUFFER_SAVED_RENDER_STATE_BIT, state);
	queue.dispatch(Queue::Opaque, cmd, &state);
	queue.dispatch(Queue::Transparent, cmd, &state);
}

DebugMeshInfo &Renderer::render_debug(RenderContext &context, const AABB &aabb, unsigned count)
{
	auto &debug = queue.emplace<DebugMeshInfo>(Queue::Opaque);
	debug.render = RenderFunctions::debug_mesh_render;
	debug.count = count;
	debug.colors = static_cast<vec4 *>(queue.allocate(debug.count * sizeof(vec4)));
	debug.positions = static_cast<vec3 *>(queue.allocate(debug.count * sizeof(vec3)));

	static const uint32_t pos_mask = 1u << ecast(MeshAttribute::Position);
	static const uint32_t color_mask = 1u << ecast(MeshAttribute::VertexColor);
	debug.program = suite[ecast(RenderableType::DebugMesh)].get_program(MeshDrawPipeline::Opaque, pos_mask | color_mask, 0).get();

	Hasher hasher;
	hasher.pointer(debug.program);
	debug.instance_key = hasher.get();
	debug.sorting_key = RenderInfo::get_sort_key(context, Queue::Opaque, hasher.get(), aabb.get_center());
	debug.MVP = context.get_render_parameters().view_projection;
	return debug;
}

void Renderer::render_debug_aabb(RenderContext &context, const AABB &aabb, const vec4 &color)
{
	auto &debug = render_debug(context, aabb, 12 * 2);

	for (unsigned i = 0; i < debug.count; i++)
		debug.colors[i] = color;

	auto *pos = debug.positions;
	*pos++ = aabb.get_coord(0.0f, 0.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 0.0f, 0.0f);

	*pos++ = aabb.get_coord(0.0f, 1.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 1.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 1.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 1.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 1.0f, 0.0f);

	*pos++ = aabb.get_coord(0.0f, 0.0f, 0.0f);
	*pos++ = aabb.get_coord(0.0f, 1.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 0.0f);
	*pos++ = aabb.get_coord(1.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(1.0f, 1.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 0.0f, 1.0f);
	*pos++ = aabb.get_coord(0.0f, 1.0f, 1.0f);
}

void Renderer::push_renderables(RenderContext &context, const VisibilityList &visible)
{
	for (auto &vis : visible)
	{
		vis.renderable->get_render_info(context, vis.transform, queue);
		if (vis.transform)
			render_debug_aabb(context, vis.transform->world_aabb, vec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
}
}