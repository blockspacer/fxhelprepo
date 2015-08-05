#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <windows.h>

// Surround接口
class ISurround
{
public:
	// 设置环绕强度(0~1.0)
	virtual void SetSurround(float fValue) = 0;

public:
	// 设置采样率
	virtual void SetSamplingRate(int nSamplingRate) = 0;

	// 设置声道数量
	virtual void SetChannels(int nChannels) = 0;

	// 复位效果器，当歌曲Seek或切换歌曲时调用
	virtual void Reset() = 0;

public:
	// 处理PCM数据(32位浮点数据)
	// 输入参数：
	//    fpBuffer，PCM缓冲区
	//    nSampleCount，缓冲区内的采样数量
	// 返回值：处理后的采样数量
	virtual int Process(float *fpBuffer, int nSampleCount) = 0;

	// 处理PCM数据(16位整数数据，s16le)
	// 输入参数：
	//    spBuffer，PCM缓冲区
	//    nSampleCount，缓冲区内的采样数量
	// 返回值：处理后的采样数量。
	virtual int Process(short *spBuffer, int nSampleCount) = 0;

public:
	// 释放实例。注意：请勿直接调用，使用ReleaseSurroundInstance来释放
	virtual void Release() = 0;
};

// 创建一个环绕实例，本库支持多实例
ISurround* WINAPI CreateSurroundInstance();

// 释放环绕实例
void WINAPI ReleaseSurroundInstance(ISurround *pInstance);


// DynamicVolume接口
class IDynamicVolume
{
public:
	// 设置动态音量的控制强度(1.0~3.5，建议使用2.5)
	virtual void SetRatio(float nRatio) = 0;

	// 设置期望音量(0~1.0)
	virtual void SetVolume(float nVolume) = 0;

	// 设置音量的最大放大倍数(1.0~无穷大，建议使用8.0)
	virtual void SetMaxGainFactor(float nFactor) = 0;

public:
	// 设置采样率
	virtual void SetSamplingRate(int nSamplingRate) = 0;

	// 设置声道数量
	virtual void SetChannels(int nChannels) = 0;

	// 复位效果器，当歌曲Seek或切换歌曲时调用
	virtual void Reset() = 0;

public:
	// 处理PCM数据(32位浮点数据)
	// 输入参数：
	//    fpBuffer，PCM缓冲区
	//    nSampleCount，缓冲区内的采样数量
	// 返回值：处理后的采样数量
	virtual int Process(float *fpBuffer, int nSampleCount) = 0;

	// 处理PCM数据(16位整数数据，s16le)
	// 输入参数：
	//    spBuffer，PCM缓冲区
	//    nSampleCount，缓冲区内的采样数量
	// 返回值：处理后的采样数量。
	virtual int Process(short *spBuffer, int nSampleCount) = 0;

public:
	// 释放实例。注意：请勿直接调用，使用ReleaseDynamicVolumeInstance来释放
	virtual void Release() = 0;
};

// 创建一个动态音量实例，本库支持多实例
IDynamicVolume* WINAPI CreateDynamicVolumeInstance();

// 释放动态音量实例
void WINAPI ReleaseDynamicVolumeInstance(IDynamicVolume *pInstance);


// Limiter接口，限幅器的用法请参考“说明.docx”
class ILimiter
{
public:
	// 复位效果器，当歌曲Seek或切换歌曲时调用
	virtual void Reset() = 0;

public:
	// 处理PCM采样(32位浮点数据)
	// 输入参数：
	//    fSample，一个32位浮点PCM采样
	// 返回值：处理后的32位浮点PCM采样
	virtual float Process(float fSample) = 0;

public:
	// 释放实例。注意：请勿直接调用，使用ReleaseLimiterInstance来释放
	virtual void Release() = 0;
};

// 创建一个限幅器实例，本库支持多实例
ILimiter* WINAPI CreateLimiterInstance();

// 释放限幅器实例
void WINAPI ReleaseLimiterInstance(ILimiter *pInstance);


#endif
