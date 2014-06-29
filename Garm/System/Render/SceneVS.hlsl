cbuffer Matrices
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexInput
{
	float4 pos : POSITION;
	float4 col : COLOR;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

VertexOut main( VertexInput inp )
{
	VertexOut result;
	inp.pos.w = 1.0f;

	result.pos = mul( inp.pos, world );
	result.pos = mul( result.pos, view );
	result.pos = mul( result.pos, projection );
	result.col = inp.col;
	return result;
}