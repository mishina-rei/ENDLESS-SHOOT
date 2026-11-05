#include "Shader.h"
#include <d3dcompiler.h>
#include <stdio.h>

#pragma comment(lib, "d3dcompiler.lib")

//----------
// ???{?N???X
Shader::Shader(Kind kind)
	: m_kind(kind)
{
}
Shader::~Shader()
{
	std::vector<ID3D11Buffer*>::iterator it = m_pBuffers.begin();
	while (it != m_pBuffers.end())
	{
		SAFE_RELEASE((*it));
		++it;
	}
}
HRESULT Shader::Load(const char* pFileName)
{
	HRESULT hr = E_FAIL;

	// ?t?@?C??????????
	FILE* fp;
	fopen_s(&fp, pFileName, "rb");
	if (!fp) { return hr; }

	// ?t?@?C????T?C?Y????
	int fileSize = 0;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);

	// ?????????????
	fseek(fp, 0, SEEK_SET);
	char* pData = new char[fileSize];
	fread(pData, fileSize, 1, fp);
	fclose(fp);

	// ?V?F?[?_?[??
	hr = Make(pData, fileSize);
	
	// ?I??????
	if (pData) { delete[] pData; }
	return hr;
}
HRESULT Shader::Compile(const char *pCode)
{
	static const char *pTargetList[] = 
	{
		"vs_5_0",
		"ps_5_0"
	};

	HRESULT hr;
	ID3DBlob *pBlob;
	ID3DBlob *error;
	UINT compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	hr = D3DCompile(pCode, strlen(pCode), nullptr, nullptr, nullptr,
		"main", pTargetList[m_kind], compileFlag, 0, &pBlob, &error);
	if (FAILED(hr)) { return hr; }

	// ?V?F?[?_??
	hr = Make(pBlob->GetBufferPointer(), (UINT)pBlob->GetBufferSize());
	SAFE_RELEASE(pBlob);
	SAFE_RELEASE(error);
	return hr;
}

void Shader::WriteBuffer(UINT slot, void* pData)
{
	if(slot < m_pBuffers.size())
		GetContext()->UpdateSubresource(m_pBuffers[slot], 0, nullptr, pData, 0, 0);
}
void Shader::SetTexture(UINT slot, Texture* tex)
{
	// if (!tex || slot >= m_pTextures.size()) { return; }
	// ID3D11ShaderResourceView* pTex = tex->GetResource();

	if (slot >= m_pTextures.size()) { return; }

	ID3D11ShaderResourceView* pTex = nullptr;
	if (tex)
		pTex = tex->GetResource();
	m_pTextures[slot] = pTex;
	switch (m_kind)
	{
	case Vertex:	GetContext()->VSSetShaderResources(slot, 1, &pTex); break;
	case Pixel:		GetContext()->PSSetShaderResources(slot, 1, &pTex); break;
	}
}
void Shader::SetTexture(UINT slot, ID3D11ShaderResourceView* pSRV)
{
	// ’Ç‰Á: ‘«‚è‚È‚¯‚ê‚ÎŠg’£‚·‚é
	if (slot >= m_pTextures.size()) {
		m_pTextures.resize(slot + 1, nullptr);
	}

	m_pTextures[slot] = pSRV;
	switch (m_kind)
	{
	case Vertex:	GetContext()->VSSetShaderResources(slot, 1, &pSRV); break;
	case Pixel:		GetContext()->PSSetShaderResources(slot, 1, &pSRV); break;
	}
}

HRESULT Shader::Make(void* pData, UINT size)
{
	HRESULT hr;
	ID3D11Device* pDevice = GetDevice();

	// ????p????t???N?V??????
	ID3D11ShaderReflection* pReflection;
	hr = D3DReflect(pData, size, IID_PPV_ARGS(&pReflection));
	if (FAILED(hr)) { return hr; }

	// ???o?b?t?@??
	D3D11_SHADER_DESC shaderDesc;
	pReflection->GetDesc(&shaderDesc);
	m_pBuffers.resize(shaderDesc.ConstantBuffers, nullptr);
	for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{
		// ?V?F?[?_?[????o?b?t?@????????ï
		D3D11_SHADER_BUFFER_DESC shaderBufDesc;
		ID3D11ShaderReflectionConstantBuffer* cbuf = pReflection->GetConstantBufferByIndex(i);
		cbuf->GetDesc(&shaderBufDesc);

		// ???????o?b?t?@?????
		D3D11_BUFFER_DESC bufDesc = {};
		bufDesc.ByteWidth = shaderBufDesc.Size;
		bufDesc.Usage = D3D11_USAGE_DEFAULT;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		// ?o?b?t?@???
		hr = pDevice->CreateBuffer(&bufDesc, nullptr, &m_pBuffers[i]);
		if (FAILED(hr)) { return hr; }
	}
	// ?e?N?X?`???????
	m_pTextures.resize(shaderDesc.TextureNormalInstructions, nullptr);

	return MakeShader(pData, size);
}

//----------
// ???_?V?F?[?_
VertexShader::VertexShader()
	: Shader(Shader::Vertex)
	, m_pVS(nullptr)
	, m_pInputLayout(nullptr)
{
}

VertexShader::~VertexShader()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVS);
}

void VertexShader::Bind(void)
{
	ID3D11DeviceContext* pContext =	GetContext();
	pContext->VSSetShader(m_pVS, NULL, 0);
	pContext->IASetInputLayout(m_pInputLayout);
	for (int i = 0; i < m_pBuffers.size(); ++i)
		pContext->VSSetConstantBuffers(i, 1, &m_pBuffers[i]);
	for (int i = 0; i < m_pTextures.size(); ++i)
		pContext->VSSetShaderResources(i, 1, &m_pTextures[i]);
}

HRESULT VertexShader::MakeShader(void* pData, UINT size)
{
	HRESULT hr;
	ID3D11Device* pDevice = GetDevice();
	
	// ?V?F?[?_?[??
	hr = pDevice->CreateVertexShader(pData, size, NULL, &m_pVS);
	if(FAILED(hr)) { return hr; }

	/*
	?V?F?[?_??????V?F?[?_???t???N?V???????????C???v?b?g???C?A?E?g???ï
	?Z?}???e?B?N?X??z?u????????q????
	????q???o?^?ƒÏ?????p?A???????V?K??
	https://blog.techlab-xe.net/dxc-shader-reflection/
	*/

	ID3D11ShaderReflection *pReflection;
	D3D11_SHADER_DESC shaderDesc;
	D3D11_INPUT_ELEMENT_DESC* pInputDesc;
	D3D11_SIGNATURE_PARAMETER_DESC sigDesc;

	DXGI_FORMAT formats[][4] =
	{
		{
			DXGI_FORMAT_R32_UINT,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R32G32B32_UINT,
			DXGI_FORMAT_R32G32B32A32_UINT,
		}, {
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32G32_SINT,
			DXGI_FORMAT_R32G32B32_SINT,
			DXGI_FORMAT_R32G32B32A32_SINT,
		}, {
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
		},
	};

	hr = D3DReflect(pData, size, IID_PPV_ARGS(&pReflection));
	if (FAILED(hr)) { return hr; }

	pReflection->GetDesc(&shaderDesc);
	pInputDesc = new D3D11_INPUT_ELEMENT_DESC[shaderDesc.InputParameters];
	for(UINT i = 0; i < shaderDesc.InputParameters; ++ i)
	{
		pReflection->GetInputParameterDesc(i, &sigDesc);
		pInputDesc[i].SemanticName = sigDesc.SemanticName;
		pInputDesc[i].SemanticIndex = sigDesc.SemanticIndex;

		// http://marupeke296.com/TIPS_No17_Bit.html
		BYTE elementCount = sigDesc.Mask;
		elementCount = (elementCount & 0x05) + ((elementCount >> 1) & 0x05);
		elementCount = (elementCount & 0x03) + ((elementCount >> 2) & 0x03);

		switch (sigDesc.ComponentType)
		{
			case D3D_REGISTER_COMPONENT_UINT32:
				pInputDesc[i].Format = formats[0][elementCount - 1];
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				pInputDesc[i].Format = formats[1][elementCount - 1];
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				pInputDesc[i].Format = formats[2][elementCount - 1];
				break;
		}
		pInputDesc[i].InputSlot = 0;
		pInputDesc[i].AlignedByteOffset = i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
		pInputDesc[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		pInputDesc[i].InstanceDataStepRate = 0;
	}

	hr = pDevice->CreateInputLayout(
		pInputDesc, shaderDesc.InputParameters,
		pData, size, &m_pInputLayout
	);
	
	delete[] pInputDesc;
	return hr;
}

//----------
// ?s?N?Z???V?F?[?_
PixelShader::PixelShader()
	: Shader(Shader::Pixel)
	, m_pPS(nullptr)
{
}
PixelShader::~PixelShader()
{
	SAFE_RELEASE(m_pPS);
}
void PixelShader::Bind(void)
{
	ID3D11DeviceContext* pContext = GetContext();
	pContext->PSSetShader(m_pPS, nullptr, 0);
	for (int i = 0; i < m_pBuffers.size(); ++i)
		pContext->PSSetConstantBuffers(i, 1, &m_pBuffers[i]);
	for (int i = 0; i < m_pTextures.size(); ++i)
		pContext->PSSetShaderResources(i, 1, &m_pTextures[i]);
}
HRESULT PixelShader::MakeShader(void* pData, UINT size)
{
	return GetDevice()->CreatePixelShader(pData, size, NULL, &m_pPS);
}