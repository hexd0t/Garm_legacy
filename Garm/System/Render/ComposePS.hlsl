Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texDepth : register(t2);
SamplerState textureSampler;

struct ComposePixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main( ComposePixelInput inp ) : SV_TARGET
{
	return (texDiffuse.Sample(textureSampler, inp.tex) * 1) 
		   //(texNormal.Sample(textureSampler, inp.tex) * 0.3);
}