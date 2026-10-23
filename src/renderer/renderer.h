#pragma once

class Mesh;
class Transform;
class ICamera;
class Shader;

class Renderer
{
public:
	Renderer();
	~Renderer();
	void beginFrame();
	void endFrame();
	void draw(const Mesh& mesh, const Transform& transform, const ICamera& camera, const Shader& shader);

private:

};