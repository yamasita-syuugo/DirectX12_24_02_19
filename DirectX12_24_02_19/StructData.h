#pragma once

//頂点データ構造体
struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};
//テクスチャデータ構造体
struct TexRGBA
{
	unsigned char R, G, B, A;
};

struct  PMDHeader
{
	float version;
	char model_name[20];
	char comment[256];
};
struct PMDVertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	uint16_t boneNo[2];
	uint8_t boneWeight;
	uint8_t ebgeFlg;
	uint16_t dummy;
};
#pragma pack(1)
struct PMDMaterial
{
	XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	XMFLOAT3 specular;
	XMFLOAT3 ambient;
	unsigned char toonIdx;
	unsigned char edgeFlg;

	unsigned int indicesNum;
	char texFilePath[20];
};
#pragma pack()
struct MaterialForHlsl
{
	XMFLOAT3 diffuse;
	float alpha;
	XMFLOAT3 specular;
	float specularity;
	XMFLOAT3 ambient;
};
struct AdditionalMaterial
{
	string texPath;
	int toonIdx;
	bool edgeFlg;
};
struct Material
{
	unsigned int indicesNum;
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

struct MatricexsData
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
};