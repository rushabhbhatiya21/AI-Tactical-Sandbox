#pragma once

class Mesh;
class Transform;
class Camera;
class Shader;

class Renderer
{
public:
	Renderer();
	~Renderer();
	void beginFrame();
	void endFrame();
	void draw(const Mesh& mesh, const Transform& transform, const Camera& camera, const Shader& shader);

private:

};