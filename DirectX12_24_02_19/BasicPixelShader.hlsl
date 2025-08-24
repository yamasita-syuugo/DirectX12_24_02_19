#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float brightness = dot(-light, input.normal);
    return float4(brightness, brightness, brightness, 1);
    
    //discard;//ピクセル破棄
    return float4(input.normal.rgb, 1);
    return float4(0, 0, 0, 1);
    return float4(tex.Sample(smp, input.uv)); //α : 1.0=不透明 0.0=透明
}