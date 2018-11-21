//-------------------------------------------------------------
//-------------------  shadow base shader ------------------------
//ʱ�䣺   2018-11-20   by:��һĨϦϼ
//˵����shadow ��shader
//-------------------------------------------------------------

#include "Common.hlsl"




struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float3 PosW : POSITION1;
    float3 NormalW : NORMAL;
    float3 TangentW : TANGENT;
    float2 TexC : TEXCOORD;
};



//������ɫ��
VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    MaterialData matData = gMaterialData[gMaterialIndex];

    //������ת��������ռ�
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

    //������������
    vout.PosW = posW.xyz;

    //��normal ��tangent ת��������ռ�
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorld);
    vout.TangentW = mul(vin.TangentU, (float3x3) gWorld);

    //������ת������βü��ռ�
    vout.PosH = mul(posW, gViewProj);

    //���ʱ任
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, matData.MatTransform).xy;

     //������ת����shadowmap�ռ� �Ա���shadowmap������ȱȽ�
    vout.ShadowPosH = mul(posW, gShadowTransform);


    return vout;
}

//������ɫ��
float4 PS(VertexOut pin):SV_Target
{
    //��ȡ������Ϣ
    MaterialData matData = gMaterialData[gMaterialIndex];
    float4 diffuseAlbedo = matData.DiffuseAlbedo;
    float3 fresnelR0 = matData.FresnelR0;
    float roughness = matData.Roughness;
    uint diffuseMapIndex = matData.DiffuseMapIndex;
    uint normalMapIndex = matData.NormalMapIndex;
	
    // ��ȡ��ͼ͸���� 
    diffuseAlbedo *= gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);

#ifdef ALPHA_TEST
   //�ü�͸���ȹ��͵�����
    clip(diffuseAlbedo.a - 0.1f);
#endif

	// �������Բ�ֵ��Ҫ���µ�λ��������
    pin.NormalW = normalize(pin.NormalW);
	
    float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    //����normalmap������
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);


    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Light terms.
    float4 ambient = gAmbientLight * diffuseAlbedo;

    //����shadowFactor
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor = CalcShadowFactor(pin.ShadowPosH);

    const float shininess = (1.0f - roughness) * normalMapSample.a;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        bumpedNormalW, toEyeW, shadowFactor,3,0,0);

    float4 litColor = ambient + directLight;

	// ���cubeMap����
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormalW, r);
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
	
    litColor.a = diffuseAlbedo.a;

    return litColor;
}