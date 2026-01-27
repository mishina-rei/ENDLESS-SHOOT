#include "RenderSystem.h"
#include "MeshRenderer.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "ShaderList.h"
#include "Sprite.h"
#include "CameraSystem.h"
#include "Defines.h"
#include "EffekseerManager.h"
#include <vector>
#include <algorithm>
#include "DirectX.h"

// ?V???h?E?}?b?v???x
const int SHADOW_MAP_WIDTH = 2048;
const int SHADOW_MAP_HEIGHT = 2048;

// static ?????o????????????`?i?????????m??j?????
Microsoft::WRL::ComPtr<ID3D11Texture2D>          RenderSystem::m_pShadowMapTexture;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView>   RenderSystem::m_pShadowMapDSV;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RenderSystem::m_pShadowMapSRV;
D3D11_VIEWPORT                                   RenderSystem::m_ShadowViewport = {}; // {} ??0?????????????????S???
Microsoft::WRL::ComPtr<ID3D11Buffer>             RenderSystem::m_pShadowCB;
Camera RenderSystem::light;

HRESULT RenderSystem::Init()
{
	light.farClip = 100.0f;

	HRESULT hr = S_OK;

	ID3D11Device* pDevice = GetDevice();

	// --------------------------------------------------
	// 1. ?e?N?X?`?????\?[?X???
	// --------------------------------------------------
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = SHADOW_MAP_WIDTH;
	texDesc.Height = SHADOW_MAP_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	// ???d?v: R24G8_TYPELESS ???w?? (?[?x24bit + ?X?e???V??8bit ?????????)
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	// ???d?v: ?[?x?o?b?t?@???????A?V?F?[?_?[???\?[?X???????g???t???O
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	hr = pDevice->CreateTexture2D(&texDesc, nullptr, m_pShadowMapTexture.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 2. Depth Stencil View (DSV) ???
	//    -> ?????[?x??????????????View
	// --------------------------------------------------
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = 0;
	// ??????[?x?t?H?[?}?b?g?????????????
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = pDevice->CreateDepthStencilView(m_pShadowMapTexture.Get(), &dsvDesc, m_pShadowMapDSV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 3. Shader Resource View (SRV) ???
	//    -> ?V?F?[?_?[??e?N?X?`?????????????View
	// --------------------------------------------------
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	// R?`?????l??(24bit?[?x????)????F?????????????
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = pDevice->CreateShaderResourceView(m_pShadowMapTexture.Get(), &srvDesc, m_pShadowMapSRV.GetAddressOf());
	if (FAILED(hr)) return hr;

	// --------------------------------------------------
	// 4. ?r???[?|?[?g????
	//    -> ?V???h?E?}?b?v??T?C?Y??????????p??Viewport???K?v
	// --------------------------------------------------
	m_ShadowViewport.TopLeftX = 0.0f;
	m_ShadowViewport.TopLeftY = 0.0f;
	m_ShadowViewport.Width = static_cast<float>(SHADOW_MAP_WIDTH);
	m_ShadowViewport.Height = static_cast<float>(SHADOW_MAP_HEIGHT);
	m_ShadowViewport.MinDepth = 0.0f;
	m_ShadowViewport.MaxDepth = 1.0f;

	// --------------------------------------------------
	// 5. Shadow Constant Buffer ??
	// --------------------------------------------------
	/*D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = sizeof(ShadowConstantBuffer);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateBuffer(&cbDesc, nullptr, m_pShadowCB.GetAddressOf());
	if (FAILED(hr)) return hr;*/

	return S_OK;
}

void RenderSystem::Draw(ECS::World* world)
{
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	GetContext()->PSSetShaderResources(
		0,
		1,
		nullSRV
	);
	// ?V???h?E?}?b?v??????????
	// ?????_?[?^?[?Q?b?g?????????A?[?x?o?b?t?@?????Z?b?g???? (?F???????????????null)
	ID3D11RenderTargetView* nullRTV = nullptr;
	GetContext()->OMSetRenderTargets(1, &nullRTV, m_pShadowMapDSV.Get());

	// ?[?x?o?b?t?@???N???A
	GetContext()->ClearDepthStencilView(m_pShadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// ?r???[?|?[?g???V???h?E?}?b?v?T?C?Y???X
	GetContext()->RSSetViewports(1, &m_ShadowViewport);

	DirectX::XMFLOAT4X4 lightView = light.GetViewMatrix(Vector3(0.0f, 10.0f, 0.0f), Quaternion(-90.0f, 0.0f, 0.0f));
	DirectX::XMFLOAT4X4 lightProj = light.GetProjectionMatrix();
	DirectX::XMFLOAT4X4 lightViewProj = DirectX::XMFLOAT4X4{};
	{
		DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&lightView);
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&lightProj);
		DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMStoreFloat4x4(&lightViewProj, viewProj);
	}

	// ?V?F?[?_?[?????o?b?t?@???u???C?g???_?v??????`??
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ???[???h?s????v?Z
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// ?V?F?[?_?[???o?b?t?@?p??s???Z?b?g
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = lightView;
		wvp[2] = lightProj;

		// ?V?F?[?_?[????
		ShaderList::SetWVP(wvp);

		//Model* pModel = mesh.pModel;

		SetDepthTest(true);  // ?[?x?e?X?g?L????

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			//???f??????b?V?????ï
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));

			// ?A?j???[?V?????p?{?[???s??????M
			if (Mesh.bones.size() > 0)
			{
				DirectX::XMFLOAT4X4 bones[200];
				for (int b = 0; b < 200; ++b) DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixIdentity());
				for (size_t b = 0; b < Mesh.bones.size() && b < 200; ++b)
				{
					const Model::Bone& bone = Mesh.bones[b];
					DirectX::XMMATRIX mat = bone.invOffset * mesh.pModel->GetBone(bone.index);
					DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixTranspose(mat));
				}
				ShaderList::SetBones(bones);
				ShaderList::GetVS(ShaderList::VS_SHADOW_ANIME)->Bind();
			}
			else
			{
				ShaderList::GetVS(ShaderList::VS_SHADOW)->Bind();
			}

			ShaderList::GetPS(ShaderList::PS_SHADOW)->Bind();

			//f?`
			Mesh.pMesh->Draw();
		}
	});
	
	// ?????`????????
	// 1. ?o?b?N?o?b?t?@(????)????
	auto p = GetDefaultRTV()->GetView();
	GetContext()->OMSetRenderTargets(1, &p, GetDefaultDSV()->GetView()); // ?????DSV

	// ?r???[?|?[?g?????
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)GetDefaultRTV()->GetWidth();
	vp.Height = (float)GetDefaultRTV()->GetHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	GetContext()->RSSetViewports(1, &vp);

	// 4. ?????????????V???h?E?}?b?v???e?N?X?`???????Z?b?g (t0 ???W?X?^)
	ShaderList::SetShadow(m_pShadowMapSRV.Get(), &lightViewProj);

	DirectX::XMFLOAT4X4 view = CameraSystem::GetView();
	DirectX::XMFLOAT4X4 proj = CameraSystem::GetProjection();
	
	//--- 3D???f?? (MeshRenderer) ??`??
	world->ForEach<MeshRenderer, Transform>([&](ECS::EntityID id, MeshRenderer& mesh, Transform& transform) {
		if (!mesh.isVisible || !mesh.pModel) return;

		// ???[???h?s????v?Z
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		// ?V?F?[?_?[???o?b?t?@?p??s???Z?b?g
		DirectX::XMFLOAT4X4 wvp[3];
		DirectX::XMStoreFloat4x4(&wvp[0], DirectX::XMMatrixTranspose(matWorld));
		wvp[1] = view;
		wvp[2] = proj;

		// ?V?F?[?_?[????
		ShaderList::SetWVP(wvp);

		//Model* pModel = mesh.pModel;

		SetDepthTest(true);  // ?[?x?e?X?g?L????

		for (int i = 0; i < mesh.pModel->GetMeshNum(); ++i) {
			//???f??????b?V?????ï
			Model::Mesh Mesh = *(mesh.pModel->GetMesh(i));
			//???b?V??????äí???????????}?e???A?????ï
			Model::Material	material = *mesh.pModel->GetMaterial(Mesh.materialID);
			//?}?e???A??????W???????ASetMaterial??????????O???X 
			//material.ambient.x = 0.85f; // x???(r)?????? 
			//material.ambient.y = 0.85f; // y???(g)?????? 
			//material.ambient.z = 0.85f; // z???(b)??????

			// MeshRenderer??F??f
			material.diffuse.x *= mesh.color.x;
			material.diffuse.y *= mesh.color.y;
			material.diffuse.z *= mesh.color.z;
			material.diffuse.w *= mesh.color.w;
			material.ambient.x *= mesh.color.x;
			material.ambient.y *= mesh.color.y;
			material.ambient.z *= mesh.color.z;
			material.ambient.w *= mesh.color.w;

			//?V?F?[?_?[??}?e???A???????
			ShaderList::SetMaterial(material);

			// ?A?j???[?V?????p?{?[???s??????M
			if (Mesh.bones.size() > 0)
			{
				DirectX::XMFLOAT4X4 bones[200];
				for (int b = 0; b < 200; ++b) DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixIdentity());
				for (size_t b = 0; b < Mesh.bones.size() && b < 200; ++b)
				{
					const Model::Bone& bone = Mesh.bones[b];
					DirectX::XMMATRIX mat = bone.invOffset * mesh.pModel->GetBone(bone.index);
					DirectX::XMStoreFloat4x4(&bones[b], DirectX::XMMatrixTranspose(mat));
				}
				ShaderList::SetBones(bones);
			}

			//???f????`??
			mesh.pModel->Draw(i);
		}
	});

	// ?G?t?F?N?g??`?? (3D???f??????A2D?X?v???C?g??O)
	EffekseerManager::Draw();

	//--- 2D?X?v???C?g (SpriteRenderer) ??`??
	// Sprite?N???X??r???[?E?v???W?F?N?V?????s???????
	Sprite::SetView(view);
	Sprite::SetProjection(proj);

	world->ForEach<SpriteRenderer,Transform>([&](ECS::EntityID id, SpriteRenderer& sprite,Transform& transform) {
		if (!sprite.isVisible || !sprite.pTexture || sprite.isUI) return;

		// ???[???h?s????v?Z
		// 3D?????z?u?????X?v???C?g??Transform??X?P?[?????g??
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(transform.position);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(transform.scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		DirectX::XMFLOAT4X4 worldOut;
		DirectX::XMStoreFloat4x4(&worldOut, DirectX::XMMatrixTranspose(matWorld));

		Sprite::SetWorld(worldOut);
		Sprite::SetTexture(sprite.pTexture.get());
		Sprite::SetColor(DirectX::XMFLOAT4(sprite.color.x, sprite.color.y, sprite.color.z, sprite.color.w));
		Sprite::SetSize(DirectX::XMFLOAT2(1.0f, 1.0f));
		Sprite::SetOffset(DirectX::XMFLOAT2((0.5f - sprite.pivot.x) * sprite.size.x, (0.5f - sprite.pivot.y) * sprite.size.y));
		Sprite::SetUVPos(DirectX::XMFLOAT2(sprite.uvPos.x, sprite.uvPos.y));
		Sprite::SetUVScale(DirectX::XMFLOAT2(sprite.uvScale.x, sprite.uvScale.y));

		Sprite::Draw();
	});

	//--- 2D?X?v???C?g(UI)???`??
	// Sprite?N???X??r???[?E?v???W?F?N?V?????s???????
	// UI?p??s???? (?????_)
	DirectX::XMFLOAT4X4 uiView;
	DirectX::XMStoreFloat4x4(&uiView, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
	DirectX::XMFLOAT4X4 uiProj;
	DirectX::XMStoreFloat4x4(&uiProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f)));

	// UI?p????
	Sprite::SetView(uiView);
	Sprite::SetProjection(uiProj);
	SetDepthTest(false);
	SetCullingMode(D3D11_CULL_NONE);

	struct UIRenderData {
		SpriteRenderer* sprite;
		Transform* transform;
	};
	std::vector<UIRenderData> uiList;

	world->ForEach<SpriteRenderer,Transform>([&](ECS::EntityID id, SpriteRenderer& sprite,Transform& transform) {
		if (!sprite.isVisible || !sprite.pTexture || !sprite.isUI) return;
		uiList.push_back({ &sprite, &transform });
	});

	std::sort(uiList.begin(), uiList.end(), [](const UIRenderData& a, const UIRenderData& b) { return a.sprite->layer < b.sprite->layer; });

	for (const auto& data : uiList) {
		SpriteRenderer& sprite = *data.sprite;
		Transform& transform = *data.transform;

		Vector3 scale = transform.scale;
		scale.y *= -1.0f; // ?????_???W?n???????Y????]

		Vector3 pos = transform.position;
		pos.z = 0.0f; // UI??Z???W??0????
		
		// ???[???h?s????v?Z
		// UI?p??X?v???C?g??sprite??transform??X?P?[?????g??
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(pos);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(transform.rotation);
		DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(scale);
		DirectX::XMMATRIX matWorld = S * R * T;

		DirectX::XMFLOAT4X4 worldOut;
		DirectX::XMStoreFloat4x4(&worldOut, DirectX::XMMatrixTranspose(matWorld));

		Sprite::SetWorld(worldOut);
		Sprite::SetTexture(sprite.pTexture.get());
		Sprite::SetColor(DirectX::XMFLOAT4(sprite.color.x, sprite.color.y, sprite.color.z, sprite.color.w));
		Sprite::SetSize(DirectX::XMFLOAT2(sprite.size.x * sprite.uvScale.x, sprite.size.y * sprite.uvScale.y));
		Sprite::SetOffset(DirectX::XMFLOAT2((0.5f - sprite.pivot.x) * sprite.size.x, (0.5f - sprite.pivot.y) * sprite.size.y));
		Sprite::SetUVPos(DirectX::XMFLOAT2(sprite.uvPos.x, sprite.uvPos.y));
		Sprite::SetUVScale(DirectX::XMFLOAT2(sprite.uvScale.x, sprite.uvScale.y));

		Sprite::Draw();
	}

	// ?`????????f?t?H???g????
	SetDepthTest(true);
	SetCullingMode(D3D11_CULL_BACK);
}