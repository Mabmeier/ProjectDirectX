#include "Model.h"
struct MODEL_CONSTANT_BUFFER
{
	XMMATRIX WorldViewProjection;
};	// TOTAL SIZE = 64 bytes;

Model::Model(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pImmediateContext) {
	m_pD3DDevice = pD3DDevice;
	m_pImmediateContext = pImmediateContext;
}
Model::~Model() {
	if (m_pObject) m_pObject->~ObjFileModel();
	if (m_pPShader) m_pPShader->Release();
	if (m_pVShader) m_pVShader->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();
}

int Model::LoadObjModel(char* filename) {
	HRESULT hr;
	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);
	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

		// Load and compile pixel and Vertex shaders - use vs_5_0 target DX11 hardware only
	ID3DBlob *VS, *PS, *error;
	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_5_0", 0, 0, 0, &VS, &error, 0);

	if (error != 0)	// check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))	// don`t fail if error is just warning
		{
			return hr;
		};
	}
	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_5_0", 0, 0, 0, &PS, &error, 0);

	if (error != 0)	// check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr))	// don`t fail if error is just warning
		{
			return hr;
		};
	}

	// Create shader objects 
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);

	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);

	if (FAILED(hr))
	{
		return hr;
	}

	// Set the shader objects as active 
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);

	// Create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);
	
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Create constant buffer
	D3D11_BUFFER_DESC constant_buffer_desc1;
	ZeroMemory(&constant_buffer_desc1, sizeof(constant_buffer_desc1));

	constant_buffer_desc1.Usage = D3D11_USAGE_DEFAULT;				// Can use UpdateSubresource() to update
	constant_buffer_desc1.ByteWidth = 64;							// MUST be a multiple of 16, calculate from CB struct
	constant_buffer_desc1.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc1, NULL, &m_pConstantBuffer);

	if (FAILED(hr)) return hr;
}

void Model::Draw(XMMATRIX* view, XMMATRIX* projection) {
	XMMATRIX world;
	world *= XMMatrixRotationX(XMConvertToRadians(m_xangle));
	world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));
	world *= XMMatrixScaling(m_scale, m_scale, m_scale);
	world = XMMatrixTranslation(m_x, m_y, m_z);

	MODEL_CONSTANT_BUFFER model_cb_values;
	model_cb_values.WorldViewProjection = world*(*view)*(*projection);

	//upload the new values for the constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);
	// set constant buffer to active
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	m_pObject->Draw();
}
void Model::LookAt_XZ(float x, float z) {
	dx = x - m_x;
	dz = z - m_z;
	m_yangle = atan2(dx, dz) * (180.0 / XM_PI);
}
void Model::MoveForward(float distance) {
	m_x += sin(m_yangle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yangle * (XM_PI / 180.0)) * distance;
}
void Model::SetXPos(float num) {
	m_x = num;
}
void Model::SetYPos(float num) {
	m_y = num;
}
void Model::SetZPos(float num) {
	m_z = num;
}
void Model::SetXAngle(float num) {
	m_xangle = num;
}
void Model::SetYAngle(float num) {
	m_yangle = num;
}
void Model::SetZAngle(float num) {
	m_zangle = num;
}
void Model::SetScale(float num) {
	m_scale = num;
}
void Model::IncXPos(float num) {
	m_x += num;
}
void Model::IncYPos(float num) {
	m_y += num;
}
void Model::IncZPos(float num) {
	m_z += num;
}
void Model::DecXPos(float num) {
	m_x -= num;
}
void Model::DecYPos(float num) {
	m_y -= num;
}
void Model::DecZPos(float num) {
	m_z -= num;
}
float Model::GetXPos() {
	return m_x;
}
float Model::GetYPos() {
	return m_y;
}
float Model::GetZPos() {
	return m_z;
}
float Model::GetXAngle() {
	return m_xangle;
}
float Model::GetYAngle() {
	return m_yangle;
}
float Model::GetZAngle() {
	return m_zangle;
}
float Model::GetScale() {
	return m_scale;
}