#ifndef _DSPINTERFACE_H
#define _DSPINTERFACE_H

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef BOOL_T
#undef BOOL_T
#endif
#define BOOL_T int

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// 音量控制方式
enum VolumeControlMethod
{
	VCM_FIXED_VOLUME   = 0,  // 固定音量衰减(默认值)
	VCM_DYNAMIC_VOLUME = 1   // 动态音量衰减
};

// 低音中心频率
enum BassCenterFrequency
{
	BCF_CENTER_55Hz = 0,  // 55Hz中心频率
	BCF_CENTER_66Hz = 1,  // 66Hz中心频率(默认值)
	BCF_CENTER_78Hz = 2   // 78Hz中心频率
};

// 接口结构
typedef struct _DSPModule
{
	// 初始化接口，当CreateDBAndCVInstance()后应该首先调用这个。返回TRUE表示成功，返回FALSE表示失败。
	//     注意：即使调用返回FALSE，依然需要调用下面的Quit()来释放接口，否则会造成内存泄露。
	//           即每一次CreateDBAndCVInstance()都要匹配一次Quit()。
	BOOL_T (*Init)(struct _DSPModule *pInstance);

	// 配置DSP模块，当Init成功后、采样率改变或需要Reset时调用。返回TRUE表示成功，返回FALSE表示采样率或声道不支持。
	//     nSamplingRate为采样率[44100、48000]，nChannels为通道数量[1、2]。
	BOOL_T (*ModuleConfig)(struct _DSPModule *pInstance, int nSamplingRate, int nChannels);

	// 是否启用动态低音。bEnable可使用的值为TRUE或FALSE。
	void (*EnableDynamicBass)(struct _DSPModule *pInstance, int bEnable);

	// 设定音量控制方式。nMode的值为VolumeControlMethod。
	void (*SetVolumeControlMethod)(struct _DSPModule *pInstance, int nMode);

	// 设定音量衰减。fValue为衰减值，范围0(不衰减)~1.0(静音)。
	void (*SetVolumeDecay)(struct _DSPModule *pInstance, float fValue);

	// 设定低音的中心频率。nCenterFrequency的值为BassCenterFrequency。
	void (*SetBassCenterFrequency)(struct _DSPModule *pInstance, int nCenterFrequency);

	// 是否启用纯净模式。bEnable可使用的值为TRUE或FALSE。
	void (*EnableClearVoice)(struct _DSPModule *pInstance, int bEnable);

	// 处理PCM数据块。fpBuffer为数据块地址，nFrames为块内的采样数量。
	//     返回值为处理后输出的采样数量。
	int (*Process)(struct _DSPModule *pInstance, float *fpBuffer, int nFrames);

	// 销毁接口并释放所有资源。每一次CreateDBAndCVInstance()都要匹配一次Quit()。
	void (*Quit)(struct _DSPModule *pInstance);

	// 接口私有数据，请勿改动或读取。
	void *pData;
} DSPModule;

// 主接口，内存不足时返回NULL。
DSPModule* WINAPI CreateDBAndCVInstance();

#ifdef __cplusplus
}
#endif

#endif
