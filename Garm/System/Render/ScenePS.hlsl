struct PixelIn
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

struct PixelOut
{
	float4 col : SV_TARGET0;
	float4 nor : SV_TARGET1;
};

PixelOut main(PixelIn inp)
{
	PixelOut result;
	result.col = inp.col;
	result.nor = inp.col;
	return result;
}