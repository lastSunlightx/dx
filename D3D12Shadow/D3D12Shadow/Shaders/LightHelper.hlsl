//----------------------------------------------//
//ʱ�䣺2018-9-29   by����һĨϦϼ
//˵�������ռ���shader
//----------------------------------------------//


#define MaxLights 16

struct Light
{
    //�����Դ������⣬���Դ���۹��
    float3 Strength;//���ջ���ǿ��
    float FalloffStart;//˥����ʼ�� spot��point����
    float3 Direction; //direction �� spot ����
    float FalloffEnd; //˥��������
    float3 Postion; //point spot ����
    float SpotPower; //spot����
};

struct Material
{
    float4 DiffuseAlbedo;//BaseColor
    float3 FresenIRO;//������0�����
    float Shininess;//�⻬�� = 1 - roughness 
};


//����˥������
float CalcAttenuation(float d,float falloffStart,float falloffEnd)
{
     // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

//������������Ʒ��̵Ľ��
float3 SchlickFresnel(float3 R0,float3 normal,float3 lightVec)
{

    //����Ϊ��R(0)=R0+(1-R0)*(cos0)^5 ����cos0ΪhalfVec��������ߵļн�
    float cosIncidentAngle = saturate(dot(normal,lightVec));

    float f0 = 1.0f - cosIncidentAngle;

    float3 reflectPrecent = R0 + (1.0f - R0) * (f0*f0*f0*f0*f0);

    return reflectPrecent;


}

//�������ͨ��
float3 BlinnPhong(float3 lightStrength,float3 lightVec,float3 normal,float3 toEye,Material mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    //����Ӱ�칫ʽ����m+8��/8 * (cos0)^m   cos0 = dot(normal,halfVec)
    float roughnessFactory = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    //����������
    float3 fresnelFactory = SchlickFresnel(mat.FresenIRO ,halfVec ,lightVec);

    float3 specAlbedo = roughnessFactory * fresnelFactory;

    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    //���ؾ��������������ú͵Ĺ�ǿ
    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;

   

}

//ƽ�й���㺯��
//����ƽ�й�ķ�����ͬ�����Թ���ǿ��ֻ��Ƕ��й���
float3 ComputDirectionalLight(Light L,Material mat,float3 normal,float3 toEye)
{
    float3 lightVec = -L.Direction;

    float lambertCos = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * lambertCos;


    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}


//���Դ���㺯��
//���Դ���������ⷽ��,��Դ������λ�ã���ͬλ����Ҫ���㣬ͬʱ��˥��Ӱ�����ǿ��
float3 ComputPointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.Postion - pos;
    float d = length(lightVec);
    if (d>L.FalloffEnd)
        return 0.0f;

    lightVec = lightVec / d;
    float lambetCos = max(dot(lightVec, normal), 0.0f);
    float att = CalcAttenuation(d,L.FalloffStart,L.FalloffEnd);//˥������

    float3 lightStrength = L.Strength * lambetCos * att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//�۹�Ƽ��㺯��
//�۹��ӵ��˥���뾶�;۹��˥������
float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.Postion - pos;
    float d = length(lightVec);
    if (d > L.FalloffEnd)
        return 0.0f;
    //��λ��lightVec�����������أ�����˥�����۹��˥��
    lightVec = lightVec / d;
    float lambetCos = max(dot(lightVec, normal), 0.0f);
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.SpotPower);

    float3 lightStrength = L.Strength * lambetCos * att * spot;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);

 

}

float4 ComputeLighting(Light gLights[MaxLights], Material mat,
                       float3 pos, float3 normal, float3 toEye,
                       float3 shadowFactor,int numDL,int numPL,int numSL)
{
    float3 result = 0.0f;

    int i = 0;
    int k = 0;

    for (i = 0; i < numDL; ++i)
    {
        result += ComputDirectionalLight(gLights[k], mat, normal, toEye) * shadowFactor;
        ++k;
    }

    for (i = 0; i < numPL; ++i)
    {
        result += ComputPointLight(gLights[k], mat, pos, normal, toEye);
        ++k;
    }

    for (i = 0; i < numSL; ++i)
    {
        result += ComputeSpotLight(gLights[k], mat, pos, normal, toEye);
        ++k;
    }


    return float4(result, 0.0f);
}


