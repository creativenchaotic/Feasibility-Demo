#include "PlaneMeshTessellated.h"
//UNUSED

PlaneMeshTessellated::PlaneMeshTessellated(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution):PlaneMesh(device, deviceContext, lresolution)
{
	resolution = lresolution;
}

PlaneMeshTessellated::~PlaneMeshTessellated()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

void PlaneMeshTessellated::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int i, j;
	float u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	vertexCount = (resolution) * (resolution) * 4;


	indexCount = vertexCount;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];


	// UV coords.
	u = 0;
	v = 0;
	increment = 1.0f / resolution+1;

	for (i = 0; i < (resolution); i++)
	{
		for (j = 0; j < (resolution); j++)
		{
			int vIndex = (i * resolution + j) * 4;//vertex index
			int iIndex = (i * resolution + j) * 4;//indices index

			// Upper left.
			vertices[vIndex].position = XMFLOAT3(i, 0.0f, j+1);
			vertices[vIndex].texture = XMFLOAT2(u, v + increment);
			vertices[vIndex].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[iIndex] = vIndex;


			// Bottom left
			vertices[vIndex].position = XMFLOAT3(i, 0.0f, j);
			vertices[vIndex].texture = XMFLOAT2(u, v);
			vertices[vIndex].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[iIndex+1] = vIndex + 1;

			// Bottom right
			vertices[vIndex].position = XMFLOAT3(i+1, 0.0f, j);
			vertices[vIndex].texture = XMFLOAT2(u + increment, v);
			vertices[vIndex].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[iIndex+2] = vIndex+2;

			// Upper right.
			vertices[vIndex].position = XMFLOAT3(i+1, 0.0f, j+1);
			vertices[vIndex].texture = XMFLOAT2(u + increment, v + increment);
			vertices[vIndex].normal = XMFLOAT3(0.0, 1.0, 0.0);
			indices[iIndex+3] = vIndex+3;

			v += increment;

		}

		v = 0;
		u += increment;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}
