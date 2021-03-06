﻿/*Copyright reserved by KenLee@2018 hellokenlee@163.com*/
#ifdef NENE_GL

#include "RenderTarget.h"
#include "Debug.h"

using namespace std;


RenderTarget::~RenderTarget() {
	if (mFBO != 0) {
		glDeleteBuffers(1, &mFBO);
	}
}

shared_ptr<RenderTarget> RenderTarget::Create(const NNUInt& width, const NNUInt& height, const NNUInt& count)
{
	return Create(width, height, count, NNPixelFormat::R8G8B8A8_UNORM);
}

shared_ptr<RenderTarget> RenderTarget::Create(const NNUInt& width, const NNUInt& height, const NNUInt& count, const NNPixelFormat& format)
{
	// 
	RenderTarget *ret = new RenderTarget();
	ret->format = format;
	ret->mWidth = width;
	ret->mHeight = height;
	// Framebuffer
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	ret->mFBO = FBO;
	// Color textures
	for (unsigned int i = 0; i < count; ++i)
	{
		ret->mColorTexes.push_back(Texture2D::CreateFromMemory(width, height, format));
	}
	// DepthStencil textures
	ret->mDepthStencilTex = Texture2D::CreateFromMemory(width, height, NNPixelFormat::D24S8_UNORM);
	// 
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	// Bind as color attacments
	vector<unsigned int> attachments;
	for (unsigned int i = 0; i < count; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, ret->mColorTexes[i]->mTextureID, 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	// Bind as depthstencil attacments
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, ret->mDepthStencilTex->mTextureID, 0);
	// 
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		dLog("[Error] Framebuffer init faild!");
		delete ret;
		return nullptr;
	}
	// Check if multi render targets is used
	glDrawBuffers((NNUInt)attachments.size(), attachments.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 
	return shared_ptr<RenderTarget>(ret);
}



void RenderTarget::Blit(const RenderTarget& src, const RenderTarget& dest, NNUInt field, NNUInt filter) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.mFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.mFBO);
	glDrawBuffer(GL_BACK);
	glBlitFramebuffer(0, 0,src.mWidth, src.mHeight, 0, 0, dest.mWidth, dest.mHeight, field, filter);
}

shared_ptr<RenderTarget> RenderTarget::CreateMultisample(const NNUInt& width, const NNUInt& height, const NNUInt& samples, const NNUInt& count) {
	// 构造对象
	RenderTarget *ret = new RenderTarget();
	ret->mWidth = width;
	ret->mHeight = height;
	// 帧缓冲对象
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	ret->mFBO = FBO;
	// 颜色附件
	for (unsigned int i = 0; i < count; ++i) {
		ret->mColorTexes.push_back(Texture2D::CreateMultisample(width, height, samples, GL_RGBA8));
	}
	// 深度模板附件
	ret->mDepthStencilTex = Texture2D::CreateMultisample(width, height, samples, GL_DEPTH24_STENCIL8);
	// 绑定附件
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	// 颜色
	vector<unsigned int> attachments;
	for (unsigned int i = 0; i < count; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, ret->mColorTexes[i]->mTextureID, 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}
	// 深度模板
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, ret->mDepthStencilTex->mTextureID, 0);
	// 检查
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		dLog("[Error] Framebuffer init faild!");
		delete ret;
		return nullptr;
	}
	// 绑定渲染对象
	glDrawBuffers(3, attachments.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 返回
	return shared_ptr<RenderTarget>(ret);
}

const shared_ptr<Texture2D> RenderTarget::GetColorTex(const NNUInt& idx) {
	if (idx < mColorTexes.size()) {
		return mColorTexes[idx];
	}
	return nullptr;
}

const shared_ptr<Texture2D> RenderTarget::GetDepthStencilTex() {
	return mDepthStencilTex;
}

void RenderTarget::Begin()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glViewport(0, 0, mWidth, mHeight);
}

void RenderTarget::End()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Utils::GetWindowWidth(), Utils::GetWindowHeight());
}

void RenderTarget::SavePixelData(const NNChar* filepath)
{
	//
	NNByte* buffer = new NNByte[mWidth * mHeight * 4 * sizeof(NNByte)];
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	{
		glReadPixels(0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//
	shared_ptr<NNByte[]> bits = shared_ptr<NNByte[]>(buffer);
	Texture::SaveImage(bits, mWidth, mHeight, format, filepath);
}


#endif // NENE_GL
