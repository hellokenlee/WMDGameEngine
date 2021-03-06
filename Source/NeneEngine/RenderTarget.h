﻿/*Copyright reserved by KenLee@2018 hellokenlee@163.com*/
#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "Texture2D.h"
#include "Types.h"
#include "Pixel.h"
#include <vector>
#include <memory>

//
//    RenderTarget: 
//

class RenderTarget
{
public:
	//
	static std::shared_ptr<RenderTarget> Create(const NNUInt& width, const NNUInt& height, const NNUInt& count);
	static std::shared_ptr<RenderTarget> Create(const NNUInt& width, const NNUInt& height, const NNUInt& count, const NNPixelFormat& format);
	static std::shared_ptr<RenderTarget> CreateMultisample(const NNUInt& width, const NNUInt& height, const NNUInt& samples, const NNUInt& count);
	//
	static void Blit(const RenderTarget& src, const RenderTarget& dest, NNUInt field, NNUInt filter);
	//
	virtual ~RenderTarget();
	//
	const std::shared_ptr<Texture2D> GetColorTex(const NNUInt& idx);
	const std::shared_ptr<Texture2D> GetDepthStencilTex();
	//
	void SavePixelData(const NNChar* filepath);
	//
	virtual void Begin();
	virtual void End();
protected:
	//
	NNPixelFormat format;
	//
	NNUInt mCount;
	NNUInt mWidth, mHeight;
	//
	std::shared_ptr<Texture2D> mDepthStencilTex;
	std::vector<std::shared_ptr<Texture2D>> mColorTexes;
#if defined NENE_GL
	GLuint mFBO;
#elif defined NENE_DX
	//
	std::vector<ID3D11RenderTargetView*> mRTVs;
	ID3D11DepthStencilView* mpDSV;
	//
	NNUInt mPreRTVCount;
	ID3D11RenderTargetView* mpPrevRTV;
	ID3D11DepthStencilView* mpPrevDSV;
#endif
protected:
	RenderTarget() = default;
	RenderTarget(const RenderTarget& rhs) = delete;
	RenderTarget& operator=(const RenderTarget& rhs) = delete;
};

#endif // RENDER_TARGET_H