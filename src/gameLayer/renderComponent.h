#pragma once

class Mesh;
class Material;

class RenderComponent
{
public:
	RenderComponent(const Mesh& mesh, const Material& material);
	const Mesh& mesh;
	const Material& material;
};
