#pragma once
//NOT USED - If expanded would be used for tessallation
#include "PlaneMesh.h"
class PlaneMeshTessellated :
    public PlaneMesh
{
public:
	PlaneMeshTessellated(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution = 100);
	~PlaneMeshTessellated();

protected:
	void initBuffers(ID3D11Device* device);
	int resolution;

};

