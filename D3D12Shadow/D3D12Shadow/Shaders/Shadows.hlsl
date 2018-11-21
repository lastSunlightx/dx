
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	MaterialData matData = gMaterialData[gMaterialIndex];
	
    //ת��������ռ�
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

    // ת������βü��ռ䣨�������ﲻ���вü�������
    vout.PosH = mul(posW, gViewProj);
	
	// ���ʶ���
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;
	
    return vout;
}

void PS(VertexOut pin) 
{
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
	
	diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);

#ifdef ALPHA_TEST
    
    //����͸����С��0.1�����壬����Ӧ�þ������clip���ԣ������Ϳ��Ծ����˳�shader
    clip(diffuseAlbedo.a - 0.1f);
#endif
}


