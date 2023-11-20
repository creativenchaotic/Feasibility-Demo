#pragma once

//Simple shader to create a fake sun
#include "DXF.h"

using namespace std;
using namespace DirectX;

class SunShader : public BaseShader
{
public:
	SunShader(ID3D11Device* device, HWND hwnd);
	~SunShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
	void initShader(const wchar_t* cs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
};

