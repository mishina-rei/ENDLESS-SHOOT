#include "Geometory.h"

void Geometory::MakeBox()
{
	//--- 頂点データの作成
	// Geometory.hで定義されているVertex(pos, uv)を使用し、
	// シェーダー(MakeVS)の入力レイアウトと一致させる
	Vertex vtx[] = {
		// 前 (Z-)
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},

		// 後 (Z+)
		{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},

		// 左 (X-)
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f}},

		// 右 (X+)
		{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},

		// 上 (Y+)
		{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},

		// 下 (Y-)
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
	};

	//--- インデックスデータの作成
	// 6面 * 2ポリゴン * 3頂点 = 36インデックス
	unsigned long idx[36];
	for (int i = 0; i < 6; ++i)
	{
		int vOffset = i * 4;
		int iOffset = i * 6;
		idx[iOffset + 0] = vOffset + 0;
		idx[iOffset + 1] = vOffset + 1;
		idx[iOffset + 2] = vOffset + 2;
		idx[iOffset + 3] = vOffset + 2;
		idx[iOffset + 4] = vOffset + 1;
		idx[iOffset + 5] = vOffset + 3;
	}

	// バッファの作成
	MeshBuffer::Description desc = {};
	desc.pVtx = vtx;
	desc.vtxSize = sizeof(Vertex);
	desc.vtxCount = _countof(vtx);
	desc.pIdx = idx;
	desc.idxSize = sizeof(unsigned long);
	desc.idxCount = _countof(idx);
	desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pBox = new MeshBuffer();
	m_pBox->Create(desc);
}

void Geometory::MakeCylinder()
{
	//--- 頂点データの作成
	// 側面、蓋

	// 設定

	//--- インデックスデータの作成
	// 側面、蓋

	// 設定


	//--- バッファの作成
}

void Geometory::MakeSphere()
{
	//--- 頂点データの作成

	//--- インデックスデータの作成

	// バッファの作成
}