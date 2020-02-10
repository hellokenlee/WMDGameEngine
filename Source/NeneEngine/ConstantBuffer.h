﻿/*Copyright reserved by KenLee@2018 hellokenlee@163.com*/
#ifndef CONSTANT_BUFFER_H
#define CONSTANT_BUFFER_H

#include "Utils.h"

//
//  ConstantBuffer: Manage Uniform Buffer Object in GL or Constant Buffer in D3D
//

template<typename T>
class ConstantBuffer {
public:
	// 构造
	ConstantBuffer();
	// 析构
	~ConstantBuffer();
	// 更新数据
	void Update(const NNUInt& slot);
public:
	// 常量缓冲数据
	T data;
private:
	// 禁止拷贝
	ConstantBuffer(const ConstantBuffer& rhs);
	ConstantBuffer& operator=(const ConstantBuffer& rhs);
	// 缓冲指针
#if defined NENE_GL
	GLuint mUBO;
#elif defined NENE_DX
	ID3D11Buffer* mpBuffer;
#endif
};

// 常量缓冲 DX 实现
#include "ConstantBuffer_DX.inl"
// 常量缓冲 GL 实现
#include "ConstantBuffer_GL.inl"

#endif // CONSTANT_BUFFER_H