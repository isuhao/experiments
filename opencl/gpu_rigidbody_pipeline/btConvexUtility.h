
/*
Copyright (c) 2012 Advanced Micro Devices, Inc.  

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
//Originally written by Erwin Coumans

#ifndef _BT_CONVEX_UTILITY_H
#define _BT_CONVEX_UTILITY_H

#include "LinearMath/btAlignedObjectArray.h"
#include "LinearMath/btTransform.h"

#include "../gpu_rigidbody_pipeline2/ConvexPolyhedronCL.h"


struct btFace
{
	btAlignedObjectArray<int>	m_indices;
	btScalar	m_plane[4];
};

class btConvexUtility
{
	public:
		
	btAlignedObjectArray<btVector3>	m_vertices;
	btAlignedObjectArray<btFace>	m_faces;
	btAlignedObjectArray<btVector3> m_uniqueEdges;

	btVector3		m_localCenter;
	btVector3		m_extents;
	btScalar		m_radius;
	btVector3		mC;
	btVector3		mE;

		
	btConvexUtility()
	{
	}
	virtual ~btConvexUtility();

	bool	initializePolyhedralFeatures(const btAlignedObjectArray<btVector3>& orgVertices, bool mergeCoplanarTriangles);
		
	void	initialize();
	bool testContainment() const;



};
#endif
	