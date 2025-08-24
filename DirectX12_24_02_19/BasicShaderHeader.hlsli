struct Output
{
    float4 svpos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

//cbuffer cbuff0 : register(b0) //定数バッファー
//{
//    matrix mat; //変換行列
//};
cbuffer cbuff0 : register(b0) //定数バッファー
{
    matrix world; //ワールド変換行列
    matrix viewproj; //ヴュープロジェクション行列
};
//struct Matrix
//{
//    matrix mat;
//};
//ConstantBuffer<Matrix> m : register(b0);