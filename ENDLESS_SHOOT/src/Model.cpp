#include "Model.h"
#include "DirectXTex/TextureLoad.h"
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#ifdef _DEBUG
#include "Geometory.h"
#endif

#if _MSC_VER >= 1930
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc143-mtd.lib")
#else
#pragma comment(lib, "assimp-vc143-mt.lib")
#endif
#elif _MSC_VER >= 1920
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif
#elif _MSC_VER >= 1910
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc141-mtd.lib")
#else
#pragma comment(lib, "assimp-vc141-mt.lib")
#endif
#endif

// staticメンバ変数の実体定義
VertexShader*	Model::m_pDefVS		= nullptr;
PixelShader*	Model::m_pDefPS		= nullptr;
unsigned int	Model::m_shaderRef	= 0;
#ifdef _DEBUG
std::string		Model::m_errorStr	= "";
#endif

/*
* @brief assimpの行列をXMMATRIX型に変換する
* @param[in] M assimpの行列
* @return 変換後の行列
*/
DirectX::XMMATRIX GetMatrixFromAssimpMatrix(aiMatrix4x4 M)
{
	return DirectX::XMMatrixSet(
		M.a1, M.b1, M.c1, M.d1,
		M.a2, M.b2, M.c2, M.d2,
		M.a3, M.b3, M.c3, M.d3,
		M.a4, M.b4, M.c4, M.d4
	);
}

/*
* @brief デフォルトのシェーダーを作成する
* @param[out] vs 頂点シェーダーへのポインタ
* @param[out] ps ピクセルシェーダーへのポインタ
*/
void MakeModelDefaultShader(VertexShader** vs, PixelShader** ps)
{
	const char* ModelVS = R"EOT(
struct VS_IN {
	float3 pos : POSITION0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
struct VS_OUT {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
VS_OUT main(VS_IN vin) {
	VS_OUT vout;
	vout.pos = float4(vin.pos, 1.0f);
	vout.pos.z += 0.5f;
	vout.pos.y -= 0.8f;
	vout.normal = vin.normal;
	vout.uv = vin.uv;
	return vout;
})EOT";
	const char* ModelPS = R"EOT(
struct PS_IN {
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};
Texture2D tex : register(t0);
SamplerState samp : register(s0);
float4 main(PS_IN pin) : SV_TARGET
{
	return tex.Sample(samp, pin.uv);
})EOT";
	*vs = new VertexShader();
	(*vs)->Compile(ModelVS);
	*ps = new PixelShader();
	(*ps)->Compile(ModelPS);
}



/*
* @brief コンストラクタ
*/
Model::Model()
	: m_loadScale(1.0f)
	, m_loadFlip(None)
	, m_playNo(ANIME_NONE)
	, m_blendNo(ANIME_NONE)
	, m_parametric{ANIME_NONE, ANIME_NONE}
	, m_blendTime(0.0f)
	, m_blendTotalTime(0.0f)
	, m_parametricBlend(0.0f)
{
	// デフォルトシェーダーの適用
	if (m_shaderRef == 0)
	{
		MakeModelDefaultShader(&m_pDefVS, &m_pDefPS);
	}
	m_pVS = m_pDefVS;
	m_pPS = m_pDefPS;
	++m_shaderRef;
}

/*
* @brief デストラクタ
*/
Model::~Model()
{
	Reset();
	--m_shaderRef;
	if (m_shaderRef <= 0)
	{
		delete m_pDefPS;
		delete m_pDefVS;
	}
}

/*
* @brief 読み込みデータの解放
*/
void Model::Reset()
{
	auto meshIt = m_meshes.begin();
	while (meshIt != m_meshes.end())
	{
		if (meshIt->pMesh) delete meshIt->pMesh;
		++meshIt;
	}

	auto matIt = m_materials.begin();
	while (matIt != m_materials.end())
	{
		if (matIt->pTexture) delete matIt->pTexture;
		++matIt;
	}
}

/*
* @brief 頂点シェーダーのセット
*/
void Model::SetVertexShader(VertexShader* vs)
{
	m_pVS = vs;
}

/*
* @brief ピクセルシェーダーのセット
*/
void Model::SetPixelShader(PixelShader* ps)
{
	m_pPS = ps;
}

/*
* @brief モデルデータの読み込み
* @param[in] file 読み込むファイルパス
* @param[in] scale モデルのサイズ倍率
* @param[in] flip 反転設定
* @return 読み込み成功でtrue
*/
bool Model::Load(const char* file, float scale, Flip flip)
{
#ifdef _DEBUG
	m_errorStr = "";
#endif
	Reset();

	// assimp初期化
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FlipUVs;
	//flag |= aiProcess_MakeLeftHanded;

	// assimpファイル読み込み
	const aiScene* pScene = importer.ReadFile(file, flag);
	if (!pScene) {
#ifdef _DEBUG
		m_errorStr = importer.GetErrorString();
#endif
		return false;
	}

	// 読み込み設定を保存
	m_loadScale = scale;
	m_loadFlip = flip;

	// ディレクトリパスの取得
	std::string directory = file;
	auto strIt = directory.begin();
	while (strIt != directory.end()) {
		if (*strIt == '/')
			*strIt = '\\';
		++strIt;
	}
	directory = directory.substr(0, directory.find_last_of('\\') + 1);

	// ノード作成
	MakeBoneNodes(pScene);
	// メッシュ作成
	MakeMesh(pScene, scale, flip);
	// マテリアル作成
	MakeMaterial(pScene, directory);

	return true;
}

/*
* @brief 描画
* @param[in] meshNo 描画するメッシュ番号、-1で全描画
*/
void Model::Draw(int meshNo)
{
	// シェーダーのバインド
	m_pVS->Bind();
	m_pPS->Bind();

	// テクスチャの自動設定フラグ
	bool isAutoTexture = (meshNo == -1);

	// 描画ループ数
	size_t drawNum = m_meshes.size();
	if (meshNo != -1)
		drawNum = meshNo + 1;
	else
		meshNo = 0;


	// 描画ループ
	for (UINT i = meshNo; i < drawNum; ++i)
	{
		if (isAutoTexture) {
			m_pPS->SetTexture(0, m_materials[m_meshes[i].materialID].pTexture);
		}
		m_meshes[i].pMesh->Draw();
	}
}

/*
* @brief メッシュ情報の取得
* @param[in] index メッシュ番号
* @return 該当するメッシュ情報
*/
const Model::Mesh* Model::GetMesh(unsigned int index)
{
	if (index < GetMeshNum())
	{
		return &m_meshes[index];
	}
	return nullptr;
}

/*
* @brief メッシュ数の取得
*/
uint32_t Model::GetMeshNum()
{
	return static_cast<uint32_t>(m_meshes.size());
}

/*
* @brief マテリアル情報の取得
* @param[in] index マテリアル番号
* @return 該当するマテリアル情報
*/
const Model::Material* Model::GetMaterial(unsigned int index)
{
	if (index < GetMaterialNum())
	{
		return &m_materials[index];
	}
	return nullptr;
}

/*
* @brief マテリアル数の取得
*/
uint32_t Model::GetMaterialNum()
{
	return static_cast<uint32_t>(m_materials.size());
}

/*
* @brief アニメーション姿勢行列の取得
* @param[in] index ボーン番号
* @return 該当するボーンの行列
*/
DirectX::XMMATRIX Model::GetBone(NodeIndex index)
{
	if (index < m_nodes.size())
	{
		return m_nodes[index].mat;
	}
	return DirectX::XMMatrixIdentity();
}

/*
* @brief アニメーション情報の取得
* @param[in] no アニメーション番号
* @return 該当するアニメーション情報
*/
const Model::Animation* Model::GetAnimation(AnimeNo no)
{
	if (AnimeNoCheck(no))
	{
		return &m_animes[no];
	}
	return nullptr;
}


/*
* @brief アニメーションの追加読み込み
* @param[in] file 読み込むアニメーションファイルパス
* @return 追加されたアニメーション番号
*/
Model::AnimeNo Model::AddAnimation(const char* file)
{
#ifdef _DEBUG
	m_errorStr = "";
#endif

	// assimp初期化
	Assimp::Importer importer;
	int flag = 0;
	flag |= aiProcess_Triangulate;
	flag |= aiProcess_FlipUVs;
	if (m_loadFlip == Flip::XFlip)  flag |= aiProcess_MakeLeftHanded;

	// assimpファイル読み込み
	const aiScene* pScene = importer.ReadFile(file, flag);
	if (!pScene)
	{
#ifdef _DEBUG
		m_errorStr += importer.GetErrorString();
#endif
		return ANIME_NONE;
	}

	// アニメーションのチェック
	if (!pScene->HasAnimations())
	{
#ifdef _DEBUG
		m_errorStr += "no animation.";
#endif
		return ANIME_NONE;
	}

	// アニメーションデータの確保
	aiAnimation* assimpAnime = pScene->mAnimations[0];
	m_animes.push_back(Animation());
	Animation& anime = m_animes.back();

	// アニメーションの解析
	float animeFrame = static_cast<float>(assimpAnime->mTicksPerSecond);
	anime.totalTime = static_cast<float>(assimpAnime->mDuration )/ animeFrame;
	anime.channels.resize(assimpAnime->mNumChannels);
	Channels::iterator channelIt = anime.channels.begin();
	while (channelIt != anime.channels.end())
	{
		// チャンネル名からノード(ボーン)を検索
		uint32_t channelIdx = static_cast<uint32_t>(channelIt - anime.channels.begin());
		aiNodeAnim* assimpChannel = assimpAnime->mChannels[channelIdx];
		Model::Nodes::iterator nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
			[assimpChannel](Node& node) {
				return node.name == assimpChannel->mNodeName.data;
			});
		if (nodeIt == m_nodes.end())
		{
			channelIt->index = INDEX_NONE;
			++ channelIt;
			continue;
		}

		// 各キーの値を取得
		channelIt->index = static_cast<NodeIndex>(nodeIt - m_nodes.begin());
		Timeline& timeline = channelIt->timeline;

		// 一旦XMVECTOR型で保存
		using XMVectorKey = std::pair<float, DirectX::XMVECTOR>;
		using XMVectorKeys = std::map<float, DirectX::XMVECTOR>;
		XMVectorKeys keys[3];
		// 位置
		for (UINT i = 0; i < assimpChannel->mNumPositionKeys; ++i)
		{
			aiVectorKey& key = assimpChannel->mPositionKeys[i];
			keys[0].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
					DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, 0.0f)
			));
		}
		// 回転
		for (UINT i = 0; i < assimpChannel->mNumRotationKeys; ++i)
		{
			aiQuatKey& key = assimpChannel->mRotationKeys[i];
			keys[1].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
				DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w)));
		}
		// 拡大縮小
		for (UINT i = 0; i < assimpChannel->mNumScalingKeys; ++i)
		{
			aiVectorKey& key = assimpChannel->mScalingKeys[i];
			keys[2].insert(XMVectorKey(static_cast<float>(key.mTime) / animeFrame,
				DirectX::XMVectorSet(key.mValue.x, key.mValue.y, key.mValue.z, 0.0f)));
		}

		// 各タイムラインのイテレータを参照用に用意
		XMVectorKeys::iterator it[] = {keys[0].begin(), keys[1].begin(), keys[2].begin()};
		for (int i = 0; i < 3; ++i)
		{
			// キーが1つしかない場合は、参照を終了
			if (keys[i].size() == 1)
				++ it[i];
		}

		// 各要素のタイムラインを統合して、1つのタイムラインにする
		while (it[0] != keys[0].end() && it[1] != keys[1].end() && it[2] != keys[2].end())
		{
			// 一番近いキー位置の時間を取得
			float time = anime.totalTime;
			for (int i = 0; i < 3; ++i)
			{
				if (it[i] != keys[i].end())
				{
					time = std::min(it[i]->first, time);
				}
			}

			// その時間での値を計算
			DirectX::XMVECTOR result[3];
			for (int i = 0; i < 3; ++i)
			{
				// 最初のキーより前の時間なら、最初の値にする
				if (time < keys[i].begin()->first)
				{
					result[i] = keys[i].begin()->second;
				}
				// 最後のキーより後の時間なら、最後の値にする
				else if (keys[i].rbegin()->first <= time)
				{
					result[i] = keys[i].rbegin()->second;
					it[i] = keys[i].end();
				}
				// キーとキーの間なら、補間値を計算
				else
				{
					// 参照時間が今の時間より小さければ、次の参照キーに進む
					if (it[i]->first <= time)
					{
						++it[i];
					}

					// 補間値の計算
					XMVectorKeys::iterator prev = it[i];
					--prev;
					float rate = (time - prev->first) / (it[i]->first - prev->first);
					result[i] = DirectX::XMVectorLerp(prev->second, it[i]->second, rate);
				}
			}

			// 姿勢情報をまとめてキーにする
			Transform transform;
			DirectX::XMStoreFloat3(&transform.translate, result[0]);
			DirectX::XMStoreFloat4(&transform.quaternion, result[1]);
			DirectX::XMStoreFloat3(&transform.scale, result[2]);
			timeline.insert(Key(time, transform));
		}

		++ channelIt;
	}

	// アニメーション番号を返す
	return static_cast<AnimeNo>(m_animes.size() - 1);
}

/*
* @brief アニメーションの更新処理
* @param[in] tick アニメーションの経過時間
*/
void Model::Step(float tick)
{
	// アニメーションの再生確認
	if (m_playNo == ANIME_NONE) { return; }

	//--- アニメーションの姿勢の更新
	// パラメトリック
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		CalcAnime(PARAMETRIC0, m_parametric[0]);
		CalcAnime(PARAMETRIC1, m_parametric[1]);
	}
	// メインアニメ
	if (m_playNo != ANIME_NONE && m_playNo != PARAMETRIC_ANIME)
	{
		CalcAnime(MAIN, m_playNo);
	}
	// ブレンドアニメ
	if (m_blendNo != ANIME_NONE && m_blendNo != PARAMETRIC_ANIME)
	{
		CalcAnime(BLEND, m_blendNo);
	}

	// アニメーションの姿勢から最終的な姿勢を更新
	CalcBones(0, DirectX::XMMatrixScaling(m_loadScale, m_loadScale, m_loadScale));

	//--- アニメーションの時間の更新
	// メインアニメ
	UpdateAnime(m_playNo, tick);
	// ブレンドアニメ
	if (m_blendNo != ANIME_NONE)
	{
		UpdateAnime(m_blendNo, tick);
		m_blendTime += tick;
		if (m_blendTime <= m_blendTotalTime)
		{
			// ブレンドアニメの再生終了
			m_blendTime = 0.0f;
			m_blendTotalTime = 0.0f;
			m_playNo = m_blendNo;
			m_blendNo = ANIME_NONE;
		}
	}
	// パラメトリック
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		UpdateAnime(m_parametric[0], tick);
		UpdateAnime(m_parametric[1], tick);
	}
}

/*
* @brief アニメーションの再生
* @param[in] no 再生するアニメーション番号
* @param[in] loop ループ再生フラグ
* @param[in] speed 再生速度
*/
void Model::Play(AnimeNo no, bool loop, float speed)
{
	// 番号チェック
	if (!AnimeNoCheck(no)) { return; }
	if (m_playNo == no) { return; }

	// パラメトリックアニメーションのチェック
	if (no != PARAMETRIC_ANIME)
	{
		// 再生情報の初期化
		InitAnime(no);
		m_animes[no].isLoop = loop;
		m_animes[no].speed = speed;
	}
	else
	{
		// パラメトリックアニメーションの場合は設定中のアニメーションを初期化する
		InitAnime(m_parametric[0]);
		InitAnime(m_parametric[1]);
		m_animes[m_parametric[0]].isLoop = loop;
		m_animes[m_parametric[1]].isLoop = loop;
		SetParametricBlend(0.0f);
	}

	// 再生アニメーション番号の設定
	m_playNo = no;
}

/*
* @brief ブレンド再生
* @param[in] no アニメーション番号
* @param[in] blendTime ブレンドに掛ける時間
* @param[in] loop ループフラグ
* @param[in] speed 再生速度
*/
void Model::PlayBlend(AnimeNo no, float blendTime, bool loop, float speed)
{
	// 番号チェック
	if (!AnimeNoCheck(no)) { return; }

	// パラメトリックアニメーションのチェック
	if (no != PARAMETRIC_ANIME)
	{
		InitAnime(no);
		m_animes[no].isLoop = loop;
		m_animes[no].speed = speed;
	}
	else
	{
		// パラメトリックアニメーションの場合は設定中のアニメーションを初期化する
		InitAnime(m_parametric[0]);
		InitAnime(m_parametric[1]);
		m_animes[m_parametric[0]].isLoop = loop;
		m_animes[m_parametric[1]].isLoop = loop;
		SetParametricBlend(0.0f);
	}

	// ブレンド情報の設定
	m_blendTime = 0.0f;
	m_blendTotalTime = blendTime;
	m_blendNo = no;
}

/*
* @brief パラメトリックアニメーションの設定
* @param[in] no1 パラメトリックアニメ1
* @param[in] no2 パラメトリックアニメ2
*/
void Model::SetParametric(AnimeNo no1, AnimeNo no2)
{
	// 番号チェック
	if (!AnimeNoCheck(no1)) { return; }
	if (!AnimeNoCheck(no2)) { return; }

	// 番号の設定
	m_parametric[0] = no1;
	m_parametric[1] = no2;
	SetParametricBlend(0.0f);
}

/*
* @brief アニメーションのパラメトリックブレンド率設定
* @param[in] blendRate ブレンド率
*/
void Model::SetParametricBlend(float blendRate)
{
	// パラメトリックアニメが設定されているか確認
	if (m_parametric[0] == ANIME_NONE || m_parametric[1] == ANIME_NONE) return;

	// ブレンド率の保存
	m_parametricBlend = blendRate;

	// ブレンド率に合わせてアニメーションの再生速度を調整
	Animation& anime1 = m_animes[m_parametric[0]];
	Animation& anime2 = m_animes[m_parametric[1]];
	float blendTotalTime = anime1.totalTime * (1.0f - m_parametricBlend) + anime2.totalTime * m_parametricBlend;
	anime1.speed = anime1.totalTime / blendTotalTime;
	anime2.speed = anime2.totalTime / blendTotalTime;
}

/*
* @brief アニメーションの再生時間を変更
* @param[in] no 変更するアニメ
* @param[in] time 新しい再生時間
*/
void Model::SetAnimationTime(AnimeNo no, float time)
{
	// 番号チェック
	if (!AnimeNoCheck(no)) { return; }

	// 再生時間の変更
	Animation& anime = m_animes[no];
	anime.nowTime = time;
	while (anime.nowTime >= anime.totalTime)
	{
		anime.nowTime -= anime.totalTime;
	}
}

/*
* @brief 再生フラグの取得
* @param[in] no 確認するアニメ番号
* @return 再生中ならtrue
*/
bool Model::IsPlay(AnimeNo no)
{
	// 番号チェック
	if (!AnimeNoCheck(no)) { return false; }

	// パラメトリック指定の場合は内部のアニメで判定
	if (no == PARAMETRIC_ANIME) { no = m_parametric[0]; }

	// 再生が終了しているか
	if (m_animes[no].totalTime < m_animes[no].nowTime) { return false; }

	// 現在再生中のものと一致しているか確認
	if (m_playNo == no) { return true; }
	if (m_blendNo == no) { return true; }
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		if (m_parametric[0] == no) { return true; }
		if (m_parametric[1] == no) { return true; }
	}

	// 再生していない
	return false;
}

/*
* @brief 再生中の番号を取得
* @return アニメ番号
*/
Model::AnimeNo Model::GetPlayNo()
{
	return m_playNo;
}

/*
* @brief 再生中のブレンドアニメ番号を取得
* @return アニメ番号
*/
Model::AnimeNo Model::GetBlendNo()
{
	return m_blendNo;
}


#ifdef _DEBUG

/*
* @brief エラーメッセージの取得
* @return エラーメッセージ
*/
std::string Model::GetError()
{
	return m_errorStr;
}

/*
* @brief ボーンのデバッグ描画
*/
void Model::DrawBone()
{
	// 再帰関数
	std::function<void(int, DirectX::XMFLOAT3)> FuncDrawBone =
		[&FuncDrawBone, this](int idx, DirectX::XMFLOAT3 parent)
	{
		// 親ノードからの線分を描画
		DirectX::XMFLOAT3 pos;
		DirectX::XMStoreFloat3(&pos, DirectX::XMVector3TransformCoord(DirectX::XMVectorZero(), m_nodes[idx].mat));
		Geometory::AddLine(parent, pos, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));

		// 子ノードの描画
		auto it = m_nodes[idx].children.begin();
		while (it != m_nodes[idx].children.end())
		{
			FuncDrawBone(*it, pos);
			++it;
		}
	};

	// 描画の実行
	FuncDrawBone(0, DirectX::XMFLOAT3());
	Geometory::DrawLines();
}

#endif


void Model::MakeBoneNodes(const void* ptr)
{
	// 再帰関数でAssimpのノードを変換して追加する
	std::function<NodeIndex(aiNode*, NodeIndex, DirectX::XMMATRIX mat)> FuncAssimpNodeConvert =
		[&FuncAssimpNodeConvert, this](aiNode* assimpNode, NodeIndex parent, DirectX::XMMATRIX mat)
	{
		DirectX::XMMATRIX transform = GetMatrixFromAssimpMatrix(assimpNode->mTransformation);
		std::string name = assimpNode->mName.data;
		if (name.find("$AssimpFbx") != std::string::npos)
		{
			mat = transform * mat;
			return FuncAssimpNodeConvert(assimpNode->mChildren[0], parent, mat);
		}
		else
		{
			// Assimpのノード情報から独自クラスに格納
			Node node;
			node.name = assimpNode->mName.data;
			node.parent = parent;
			node.children.resize(assimpNode->mNumChildren);
			node.mat = mat;

			// ノードリストへ追加
			m_nodes.push_back(node);
			NodeIndex nodeIndex = static_cast<NodeIndex>(m_nodes.size() - 1);

			// 子要素の再帰処理
			for (UINT i = 0; i < assimpNode->mNumChildren; ++i)
			{
				m_nodes[nodeIndex].children[i] = FuncAssimpNodeConvert(
					assimpNode->mChildren[i], nodeIndex, DirectX::XMMatrixIdentity());
			}
			return nodeIndex;
		}
	};

	// ノード生成
	m_nodes.clear();
	FuncAssimpNodeConvert(reinterpret_cast<const aiScene*>(ptr)->mRootNode, INDEX_NONE, DirectX::XMMatrixIdentity());

	// アニメーション計算用配列、ノードの数だけデータを用意
	Transform init = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f)
	};
	for (int i = 0; i < MAX_TRANSFORM; ++i)
	{
		m_nodeTransform[i].resize(m_nodes.size(), init);
	}
}

void Model::MakeWeight(const void* ptr, int meshIdx)
{
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);

	// メッシュが持っているボーンを取得する
	aiMesh* assimpMesh = pScene->mMeshes[meshIdx];
	Mesh& mesh = m_meshes[meshIdx];
	if (assimpMesh->HasBones())
	{
		// メッシュの頂点ごとのウェイト
		struct WeightPair
		{
			unsigned int idx;
			float weight;
		};
		std::vector<std::vector<WeightPair>> weights;
		weights.resize(mesh.vertices.size());


		// メッシュが影響を受けるすべてのボーン情報を確保
		mesh.bones.resize(assimpMesh->mNumBones);
		for (auto boneIt = mesh.bones.begin(); boneIt != mesh.bones.end(); ++boneIt)
		{
			UINT boneIdx = static_cast<UINT>(boneIt - mesh.bones.begin());
			aiBone* assimpBone = assimpMesh->mBones[boneIdx];
			// 関連するボーンとノード名から該当するノードを取得
			std::string boneName = assimpBone->mName.data;
			auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
				[boneName](const Node& val) {
				return val.name == boneName;
			});
			// メッシュが影響を受けるすべてのボーンが、ノードに含まれているか
			if (nodeIt == m_nodes.end())
			{
				boneIt->index = INDEX_NONE;
				continue;
			}

			// メッシュとボーンをノードに関連付け
			boneIt->index = static_cast<NodeIndex>(nodeIt - m_nodes.begin());
			boneIt->invOffset = GetMatrixFromAssimpMatrix(assimpBone->mOffsetMatrix);
			boneIt->invOffset.r[3].m128_f32[0] *= m_loadScale;
			boneIt->invOffset.r[3].m128_f32[1] *= m_loadScale;
			boneIt->invOffset.r[3].m128_f32[2] *= m_loadScale;
			boneIt->invOffset =
				DirectX::XMMatrixScaling(m_loadFlip == ZFlipUseAnime ? -1.0f : 1.0f, 1.0f, 1.0f) *
				boneIt->invOffset * 
				DirectX::XMMatrixScaling(1.f / m_loadScale, 1.f / m_loadScale, 1.f / m_loadScale);

			// ウェイトの取得
			UINT weightNum = assimpBone->mNumWeights;
			for (UINT i = 0; i < weightNum; ++i)
			{
				aiVertexWeight weight = assimpBone->mWeights[i];
				weights[weight.mVertexId].push_back({boneIdx, weight.mWeight});
			}
		}

		// 取得した頂点・ウェイトの設定
		for (int i = 0; i < weights.size(); ++i)
		{
			if (weights[i].size() >= 4)
			{
				std::sort(weights[i].begin(), weights[i].end(), [](WeightPair& a, WeightPair& b) {
					return a.weight > b.weight;
				});
				// ウェイトが4つ以上の場合は正規化
				float total = 0.0f;
				for (int j = 0; j < 4; ++j)
					total += weights[i][j].weight;
				for (int j = 0; j < 4; ++j)
					weights[i][j].weight /= total;
			}
			for (int j = 0; j < weights[i].size() && j < 4; ++j)
			{
				mesh.vertices[i].index[j] = weights[i][j].idx;
				mesh.vertices[i].weight[j] = weights[i][j].weight;
			}
		}
	}
	else
	{
		// メッシュ親ノードのトランスフォーム行列から計算
		std::string nodeName = assimpMesh->mName.data;
		auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(),
			[nodeName](const Node& val) {
				return val.name == nodeName;
			});
		if (nodeIt == m_nodes.end())
		{
			return;	// ボーンデータなし
		}

		// メッシュの真の親ノードを、再帰検索
		std::function<int(int)> FuncFindNode =
			[&FuncFindNode, this, pScene](NodeIndex parent)
		{
			std::string name = m_nodes[parent].name;
			for (UINT i = 0; i < pScene->mNumMeshes; ++i)
			{
				if (name == pScene->mMeshes[i]->mName.data)
				{
					return FuncFindNode(m_nodes[parent].parent);
				}
			}
			return parent;
		};

		Bone bone;
		bone.index = FuncFindNode(nodeIt->parent);
		bone.invOffset = DirectX::XMMatrixInverse(nullptr, m_nodes[bone.index].mat);
		for (auto vtxIt = mesh.vertices.begin(); vtxIt != mesh.vertices.end(); ++vtxIt)
		{
			vtxIt->weight[0] = 1.0f;
		}

		mesh.bones.resize(1);
		mesh.bones[0] = bone;
	}
}



bool Model::AnimeNoCheck(AnimeNo no)
{
	// パラメトリックアニメーションの確認
	if (no == PARAMETRIC_ANIME)
	{
		// パラメトリック用アニメーションがどちらも設定されているか
		return
			m_parametric[0] != ANIME_NONE &&
			m_parametric[1] != ANIME_NONE;
	}
	else
	{
		// 通常のアニメーションが範囲内か
		return 0 <= no && no < m_animes.size();
	}
}
void Model::InitAnime(AnimeNo no)
{
	// アニメがない場合、パラメトリックが指定されている場合は処理しない
	if (no == ANIME_NONE || no == PARAMETRIC_ANIME) { return; }

	Animation& anime = m_animes[no];
	anime.nowTime = 0.0f;
	anime.speed = 1.0f;
	anime.isLoop = false;
}
void Model::CalcAnime(AnimeTransform kind, AnimeNo no)
{
	Animation& anime = m_animes[no];
	Channels::iterator channelIt = anime.channels.begin();
	while (channelIt != anime.channels.end())
	{
		// 非対象のボーンやアニメ無しはスキップ
		Timeline& timeline = channelIt->timeline;
		if (channelIt->index == INDEX_NONE || timeline.empty())
		{
			++channelIt;
			continue;
		}

		//--- 該当ノードの姿勢をアニメーションから更新
		Transform& transform = m_nodeTransform[kind][channelIt->index];
		if (timeline.size() <= 1)
		{
			// キーが一つしかない場合はその値を使用
			transform = channelIt->timeline[0];
		}
		else
		{
			Timeline::iterator startIt = timeline.begin();
			if (anime.nowTime <= startIt->first)
			{
				// 最初のキーより前なら、最初の値を使用
				transform = startIt->second;
			}
			else if (timeline.rbegin()->first <= anime.nowTime)
			{
				// 最後のキーより後なら、最後の値を使用
				transform = timeline.rbegin()->second;
			}
			else
			{
				// 指定時間を挟む2つのキーから、補間した値を計算
				Timeline::iterator nextIt = timeline.upper_bound(anime.nowTime);
				startIt = nextIt;
				--startIt;
				float rate = (anime.nowTime - startIt->first) / (nextIt->first - startIt->first);
				LerpTransform(&transform, startIt->second, nextIt->second, rate);
			}
		}

		++channelIt;
	}
}
void Model::UpdateAnime(AnimeNo no, float tick)
{
	if (no == PARAMETRIC_ANIME) { return; }

	Animation& anime = m_animes[no];
	anime.nowTime += anime.speed * tick;
	if (anime.isLoop)
	{
		while (anime.nowTime >= anime.totalTime)
		{
			anime.nowTime -= anime.totalTime;
		}
	}
}
void Model::CalcBones(NodeIndex index, const DirectX::XMMATRIX parent)
{
	//--- アニメーションから姿勢データを取得
	Transform transform;
	// パラメトリック
	if (m_playNo == PARAMETRIC_ANIME || m_blendNo == PARAMETRIC_ANIME)
	{
		LerpTransform(&transform, m_nodeTransform[PARAMETRIC0][index], m_nodeTransform[PARAMETRIC1][index], m_parametricBlend);
		if (m_playNo == PARAMETRIC_ANIME)
		{
			m_nodeTransform[MAIN][index] = transform;
		}
		if (m_blendNo == PARAMETRIC_ANIME)
		{
			m_nodeTransform[BLEND][index] = transform;
		}
	}
	// ブレンドアニメ
	if (m_blendNo != ANIME_NONE)
	{
		LerpTransform(&transform, m_nodeTransform[MAIN][index], m_nodeTransform[BLEND][index], m_blendTime / m_blendTotalTime);
	}
	else
	{
		// メインアニメのみ
		transform = m_nodeTransform[MAIN][index];
	}

	// 該当ノードの姿勢行列を計算
	Node& node = m_nodes[index];
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transform.translate));
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&transform.quaternion));
	DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transform.scale));
	node.mat = (S * R * T) * parent;

	// 子要素の姿勢を更新
	Children::iterator it = node.children.begin();
	while (it != node.children.end())
	{
		CalcBones(*it, node.mat);
		++it;
	}
}

void Model::LerpTransform(Transform* pOut, const Transform& a, const Transform& b, float rate)
{
	DirectX::XMVECTOR vec[][2] = {
		{ DirectX::XMLoadFloat3(&a.translate),	DirectX::XMLoadFloat3(&b.translate) },
		{ DirectX::XMLoadFloat4(&a.quaternion),	DirectX::XMLoadFloat4(&b.quaternion) },
		{ DirectX::XMLoadFloat3(&a.scale),		DirectX::XMLoadFloat3(&b.scale) },
	};
	for (int i = 0; i < 3; ++i)
	{
		vec[i][0] = DirectX::XMVectorLerp(vec[i][0], vec[i][1], rate);
	}
	DirectX::XMStoreFloat3(&pOut->translate, vec[0][0]);
	DirectX::XMStoreFloat4(&pOut->quaternion, vec[1][0]);
	DirectX::XMStoreFloat3(&pOut->scale, vec[2][0]);
}
