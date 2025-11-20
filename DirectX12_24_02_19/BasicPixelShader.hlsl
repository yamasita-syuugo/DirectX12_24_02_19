#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
    //光の反射ベクトル
    float3 light = normalize(float3(1, -1, 1));
    float3 lightColor = (1, 1, 1);
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1) * float2(0.5f, -0.5f));
    float4 color = tex.Sample(smp, input.uv);
    float brightness = dot(-light, input.normal);
    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
    float diffuseB = dot(-light,input.normal);
    float2 normalUV = (input.normal.xy + float2(1, -1)) * float2(0.5, -0.5);
    return max(diffuseB * diffuse * color * sph.Sample(smp,sphereMapUV) + spa.Sample(smp,sphereMapUV) + float4(specularB * specular.rgb, 1), float4(ambient * color, 1));
    
    return float4(brightness, brightness, brightness, 1) *
    diffuse *
    tex.Sample(smp, input.uv) *
    sph.Sample(smp, sphereMapUV) +
    spa.Sample(smp, sphereMapUV) +
    float4(color * ambient, 1);//アンビエント
    
    //discard;//ピクセル破棄
    return float4(input.normal.rgb, 1);
    return float4(0, 0, 0, 1);//黒
    return float4(tex.Sample(smp, input.uv)); //α : 1.0=不透明 0.0=透明
}