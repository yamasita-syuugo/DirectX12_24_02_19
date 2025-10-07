#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float brightness = dot(-light, input.normal);
    float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
    float2 sphereMapUV = input.vnormal.xy;
    return float4(brightness, brightness, brightness, 1) * 
    diffuse * 
    tex.Sample(smp, input.uv) * 
    sph.Sample(smp, sphereMapUV) +
    spa.Sample(smp, sphereMapUV);
    
    //discard;//ピクセル破棄
    return float4(input.normal.rgb, 1);
    return float4(0, 0, 0, 1);//黒
    return float4(tex.Sample(smp, input.uv)); //α : 1.0=不透明 0.0=透明
}