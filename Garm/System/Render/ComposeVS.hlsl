struct ComposeVertexInput
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};
struct ComposeVertexOutput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

ComposeVertexOutput main( ComposeVertexInput inp )
{
	ComposeVertexOutput result;
	result.pos = inp.pos;
	result.tex = inp.tex;
	return result;
}