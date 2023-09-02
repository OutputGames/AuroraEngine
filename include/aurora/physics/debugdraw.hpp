#ifndef DEBUGD_HPP
#define DEBUGD_HPP

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

struct BulletDebugDrawer : btIDebugDraw 
{
	void draw3dText(const btVector3 &, const char *) override
};


#endif