struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0);
SamplerState smp : register(s0);

cbuffer cbuff0 : register(b0) //�萔�o�b�t�@�[
{
    matrix mat; //�ϊ��s��
};
//struct Matrix
//{
//    matrix mat;
//};
//ConstantBuffer<Matrix> m : register(b0);
