struct ComposePixelInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main( ComposePixelInput inp ) : SV_TARGET
{
	return float4(0.0f, 1.0f, 1.0f, 1.0f);
}