#ifndef _VIRTUALIZER
#define _VIRTUALIZER

#include <windows.h>

// 主接口
class IVirtualizer
{
public:
	// 激活效果器，同时设定采样率、声道数和效果模型的ID（详见下面）
	// 返回false为失败
	virtual bool Active(int nSamplingRate, int nChannels, int nModelID) = 0;
	// 重置效果器
	virtual void Reset() = 0;
	// 处理PCM采样，nFrames为采样数量，返回值是处理后的采样数量
	virtual int Process(float *fpPCMBuffer, int nFrames) = 0;

public:
	// 销毁接口
	virtual void Release() = 0;
};

// 格式支持检查接口，采样率、声道数和效果模型的ID（详见下面）。返回1为支持，0为不支持
int WINAPI VirtualizerCheckFormat(int nSamplingRate, int nChannels, int nModeID);
// 创建Virtualizer实例
IVirtualizer* WINAPI CreateVirtualizer();

/*
 * 模型ID[nModeID]为效果选择的模型编号
 * 实际有1和2两个
 *
 * 使用流程为：
 * CreateVirtualizer -> VirtualizerCheckFormat ->
 * Active -> Process -> Release
 *
 * Virtualizer支持的音频格式为：
 * 采样率：44100、48000、88200、96000、176400、192000
 * 声道数：2
 * 对于单声道情况若要正常使用则需要将单声道复制一份做成伪立体声
 *
 * 注意：
 *   当调用Active时，Virtualizer会强制清空缓存、释放并重新分配一些内存
 *   同时会做大量的运算，会造成瞬间CPU和RAM占用升高。
 *   因此建议仅当采样率或模型ID发生变化时再调用。
 */

/*

使用方法：
// 初始化
{
	// 新建实例
	IVirtualizer *pVirtualizer = CreateVirtualizer();
	if (pVirtualizer == NULL) {
		MessageOut("Failed to create virtualizer instance.");
		return -1;
	}

	// 使用默认参数激活Virtualizer
	if (!pVirtualizer->Active(44100, 2, ModelID)) {
		MessageOut("Failed to active virtualizer.");
		return -1;
	}
}

// 切歌或拖动回调，即需要Reset效果器的时候
void SongSeekCallback()
{
	// 由于Virtualizer内部没有锁，因此多线程环境下需要原子操作
	EnterCriticalSection();
	// Reset效果器
	pVirtualizer->Reset();
	LeaveCriticalSection();
}

// 当音频的采样率或声道数变化时
void UpdateAudioFormat(int nSamplingRate, int nChannels)
{
	// 若采样率和声道数没有变化，则不要调用Active
	if ((nSamplingRate == nOldSamplingRate) &&
		(nChannels == nOldChannels))
		return;

	// 由于Virtualizer内部没有锁，因此多线程环境下需要原子操作
	EnterCriticalSection();
	// 检查格式支持
	if (!VirtualizerCheckFormat(nSamplingRate, nChannels, ModelID)) {
		MessageOut("Unsupported audio format.");
		LeaveCriticalSection();
		return;
	}
	// 更新格式
	if (!pVirtualizer->Active(nSamplingRate, nChannels, ModelID)) {
		MessageOut("Failed to active virtualizer.");
		LeaveCriticalSection();
		return;
	}
	LeaveCriticalSection();

	// 保存音频格式
	nOldSamplingRate = nSamplingRate;
	nOldChannels = nChannels;
}

// 处理线程
DWORD ProcessingThread(void *p)
{
	// 解码音频数据
	float faPCMSamples[BUFFER_LENGTH];
	int nFrames = GetDecoderPCM(faPCMSamples, BUFFER_LENGTH);

	// 效果处理，由于Virtualizer内部没有锁，因此多线程环境下需要原子操作
	EnterCriticalSection();
	nFrames = pVirtualizer->Process(faPCMSamples, nFrames);
	LeaveCriticalSection();

	// 后续音效处理
	EqualizerProcess(faPCMSamples, nFrames);		// 均衡器处理
	DynamicVolumeProcess(faPCMSamples, nFrames);	// 智能音量处理
	LimiterProcess(faPCMSamples, nFrames);			// 限幅器

	// 输出
	AudioOutputWrite(faPCMSamples, nFrames);
}

// 销毁实例
{
	pVirtualizer->Release();
	pVirtualizer = NULL;
}

注意：
	上面示例代码中使用了多线程环境，因此需要对pVirtualizer进行加锁。
	若实际为单线程调用，则无需加锁。

*/


#endif
