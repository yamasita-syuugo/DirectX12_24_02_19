#include"GameObject.h"

#define imageFileName ("img/001.webp")

GameObject::GameObject()
{
	Vertex vertex[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},
		{{-0.4f,0.7f,0.0f},	{0.0f,0.0f}},
		{{0.4f,-0.7f,0.0f},	{1.0f,1.0f}},
		{{0.4f,0.7f,0.0f},	{1.0f,0.0f}},
	};	
	for (int i = 0; i < sizeof Vertex / sizeof vertex[0];i++)SetVertices(i,vertex[i]);
	//インデックスの実装------------------------------------
	unsigned short index[] = {
		0,1,2,
		2,1,3,
	};
	for (int i = 0; i < sizeof (unsigned short) / sizeof index[0]; i++)SetIndex(i, index[i]);


}
void Initialize() {

}
void Loop() {

}

GameObject::~GameObject()
{
	comList->Release();
}