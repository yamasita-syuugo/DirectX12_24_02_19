#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    //discard;//ピクセル破棄
    //return float4(0, 0, 0, 1);
    return float4(tex.Sample(smp, input.uv)); //α : 1.0=不透明 0.0=透明
}