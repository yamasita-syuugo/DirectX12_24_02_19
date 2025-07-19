#pragma once
#include"UseHeader.h"

class GameObject
{
private:
	ID3D12GraphicsCommandList* comList;

private:
	Vertex vertices[4];
	unsigned short index[6];

public:
	ID3D12GraphicsCommandList* GetComList() { return comList; }
	Vertex GetVertices(int index) { return vertices[index]; }
	void SetVertices(int vertexNum, Vertex vertices_) { vertices[vertexNum] = vertices_; }
	short GetIndex(int index_) { return index[index_]; }
	void SetIndex(int num,unsigned short index_) { index[num] = index_; }

public:
	GameObject();
	~GameObject();

};