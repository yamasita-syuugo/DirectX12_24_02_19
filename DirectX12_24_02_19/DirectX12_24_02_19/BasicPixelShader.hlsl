#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    //discard;//�s�N�Z���j��
    return float4(tex.Sample(smp, input.uv)); //�� : 1.0=�s���� 0.0=����
}