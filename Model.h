#pragma once
#include "objfilemodel.h"

class Model
{
private:
	float m_x, m_y, m_z	= 0.0f;
	float m_xangle, m_zangle, m_yangle = 0.0f;
	float m_scale = 1.0f;
	ID3D11Device*			m_pD3DDevice;
	ID3D11DeviceContext*	m_pImmediateContext;
	ObjFileModel*			m_pObject;
	ID3D11VertexShader*		m_pVShader;
	ID3D11PixelShader*		m_pPShader;
	ID3D11InputLayout*		m_pInputLayout;
	ID3D11Buffer*			m_pConstantBuffer;
public:	
	Model(ID3D11Device* pD3DDevice,ID3D11DeviceContext*	pImmediateContext);
	~Model();
	int LoadObjModel(char* filename);
	void Draw(XMMATRIX* view, XMMATRIX* projection);
	void SetXPos(float num);
	void SetYPos(float num);
	void SetZPos(float num);
	void SetXAngle(float num);
	void SetYAngle(float num);
	void SetZAngle(float num);
	void SetScale(float num);
	void IncXPos(float num);
	void IncYPos(float num);
	void IncZPos(float num);
	void DecXPos(float num);
	void DecYPos(float num);
	void DecZPos(float num);
	float GetXPos();
	float GetYPos();
	float GetZPos();
	float GetXAngle();
	float GetYAngle();
	float GetZAngle();
	float GetScale();
};
