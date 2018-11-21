//-------------------------------------------------------------
//-------------------d3d12ShadowMap----------------------------
//ʱ�䣺2018-11-20       by:��һĨϦϼ
//˵����d3d12 ShadowMap Demo.
//-------------------------------------------------------------
#pragma once


#include "D3D12App.h"
#include "FrameResource.h"
#include "D3D12Camera.h"
#include "CubeRenderTarget.h"
#include "ShadowMap.h"

struct RenderItem
{
	RenderItem() = default;

	//�������
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	//Texture Transform
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	//��Ӧframeresource�еĳ���������������
	UINT ObjCBIndex = -1;

	//�������
	Material *Mat = nullptr;
	//���ü�������Ϣ
	MeshGeometry *Geo = nullptr;

	//primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

};



class ShadowMapApp :public D3D12App
{
public:
	ShadowMapApp(UINT width, UINT height, std::wstring name);
	~ShadowMapApp();
	ShadowMapApp(const ShadowMapApp& rhs) = delete;
	ShadowMapApp &operator=(const ShadowMapApp& rhs) = delete;


	virtual void OnInit() override;
private:
	//����������Ҫ����dsv����rtv�������ش˺���
	virtual void CreateRtvAndDsvDescriptorHeaps() override;
	virtual void OnResize()override;
	virtual void OnUpdate(const GameTimer& gt)override;
	virtual void OnRender(const GameTimer& gt)override;
	virtual void CalculateFrameStats() override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);

	void UpdateObjectCBs(const GameTimer& gt);//��������³�����������material������
	void UpdateMainPassCB(const GameTimer& gt);//��������¹�������������
	//������Ӱ��Ϣ
	void UpdateShadowTransform(const GameTimer& gt);
	void UpdateShadowPassCB(const GameTimer& gt);

	void LoadTextures();//��ͼ��Դ����
	void BuildRootSignature();//���ø�ǩ��
	void BuildDescriptorHeaps();//������Դ��descriptor
	void BuildShadersAndInputLayout();//����hlsl�Ķ����ʽ
	void BuildShapeGeometry();//��������������
	void BuildSkullGeometry();//����ͷ����Ϣ
	void BuildPSOs();//�����ܵ�״̬
	void BuildFrameResources();//����ҳ����Դ
	void BuildMaterials();//���ò���
	void BuildRenderItems();//�������ƶ�����Ϣ

	//����object
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
	//���Ƶƹ�������������Ϣ�� shadowmap
	void DrawSceneToShadowMap();

	//��ͼ�Ĳ�����ʽ ����ʹ�ó�����ͼ������ʽ �������ǿ���ֱ��ʹ�ø�ǩ�������õĲ�����ʽ�󶨵�hlsl
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	//����demoʹ�õ�srv��Դ��descriptor
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;//���еĶ��㻺��
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;//���еĲ���
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;//���е���ͼ
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;//���е�shader
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;//���е�PipelineState


	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	// ��Ҫ��Ⱦ����������
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// ���ֲ�ͬ�ܵ�״̬ PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	//���ڱ�ʶ���ǵ�sky ��  shadow��Դ��ƫ����
	UINT mSkyTexHeapIndex = 0;
	UINT mShadowMapHeapIndex = 0;


	PassConstants mMainPassCB;  // ��ͨ�������Ĺ�����Ϣ
	PassConstants mShadowPassCB;// �ƹ�����Ĺ�����Ϣ

	//���������
	Camera  mCamera;

	//shadowmap  for our  shadowtest
	std::unique_ptr<ShadowMap> mShadowMap;
	//���ڰ���������sphere
	DirectX::BoundingSphere mSceneBounds;

	//�����ƹ�Ϊ�ӵ����Ⱦ����Ҫ����Ϣ
	float mLightNearZ = 0.0f;
	float mLightFarZ = 0.0f;
	XMFLOAT3 mLightPosW;
	XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
	XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

	//�����ת�Ƕ�
	float mLightRotationAngle = 0.0f;
	//��ķ���
	XMFLOAT3 mBaseLightDirections[3] = {
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	XMFLOAT3 mRotatedLightDirections[3];

	POINT mLastMousePos;

};