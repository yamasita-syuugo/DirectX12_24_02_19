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