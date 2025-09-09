struct Output
{
    float4 svpos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

//t -シェーダー リソースビュー (SRV)用
//s -サンプラー用
//u -順序指定されていないアクセス ビュー(UAV) 用
//b -定数バッファー ビュー(CBV) 用

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
cbuffer Material : register(b1)
{
    float4 diffuse;
    float4 specular;
    float3 ambient;
}
//struct Matrix
//{
//    matrix mat;
//};
//ConstantBuffer<Matrix> m : register(b0);