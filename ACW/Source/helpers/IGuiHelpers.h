#pragma once

class GameEntity;
class Pipe;
class Camera;
class Ball;

class IGuiHelpers
{
public:
	virtual void RenderGui(GameEntity*) const = 0;
	virtual void RenderGui(Pipe*) const = 0;
	virtual void RenderGui(Camera*) const = 0;
	virtual void RenderGui(Ball*) const = 0;
};