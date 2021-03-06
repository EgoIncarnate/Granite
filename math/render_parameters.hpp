/* Copyright (c) 2017 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "math.hpp"
#include "image.hpp"

namespace Granite
{
struct RenderParameters
{
	mat4 projection;
	mat4 view;
	mat4 view_projection;
	mat4 inv_projection;
	mat4 inv_view;
	mat4 inv_view_projection;
	mat4 inv_local_view_projection;

	alignas(16) vec3 camera_position;
	alignas(16) vec3 camera_front;
	alignas(16) vec3 camera_right;
	alignas(16) vec3 camera_up;

	float z_near;
	float z_far;
};

struct ResolutionParameters
{
	vec2 resolution;
	vec2 inv_resolution;
};

struct FogParameters
{
	vec3 color;
	float falloff;
};

struct DirectionalParameters
{
	alignas(16) vec3 color;
	alignas(16) vec3 direction;
};

struct ShadowParameters
{
	mat4 near_transform;
	mat4 far_transform;
	float inv_cutoff_distance;
};

struct EnvironmentParameters
{
	float intensity;
	float mipscale;
};

struct RefractionParameters
{
	vec3 falloff;
};

struct LightingParameters
{
	FogParameters fog;
	DirectionalParameters directional;
	ShadowParameters shadow;
	EnvironmentParameters environment;
	RefractionParameters refraction;

	Vulkan::ImageView *environment_radiance = nullptr;
	Vulkan::ImageView *environment_irradiance = nullptr;
	Vulkan::ImageView *shadow_near = nullptr;
	Vulkan::ImageView *shadow_far = nullptr;
};
}