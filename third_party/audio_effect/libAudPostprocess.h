#ifndef _LIBAUDPOSTPROCESS_H_
#define _LIBAUDPOSTPROCESS_H_

#if defined(_MSC_VER)
#include <windows.h>
#define LINKING WINAPI
#else
#define LINKING
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*********** 音效组件ID ***********/
#define COMPONENT_ID_VOCAL_FIX      0x1001	/* 人声高频修复和降噪 */
#define COMPONENT_ID_VOCAL_HARMONIC 0x1002	/* 人声谐波处理，清晰度和暖音 */
#define COMPONENT_ID_VOCAL_AGC      0x1003	/* 人声A/C音量增益 */
#define COMPONENT_ID_VOCAL_REVERB   0x1004	/* 人声混响 */
#define COMPONENT_ID_BG_AGC         0x2001	/* 伴奏A/C音量增益 */
#define COMPONENT_ID_BG_SURROUND    0x2002	/* 伴奏3D环绕 */
#define COMPONENT_ID_BG_REVERB      0x2003	/* 伴奏混响 */
/**********************************/

// 混响预设配置ID(EAX标准)
// HQ版可全部使用，LQ版按下方注释为准
static const int REVERB_PRESET_ID[113] =
{
	0,		// 一般(默认)，LQ版可用
	1,		// 四周为软垫的小房间
	2,		// KTV房间，LQ版可用
	3,		// 浴室，LQ版可用
	4,		// 起居室
	5,		// 石头屋
	6,		// 大礼堂，LQ版可用
	7,		// 音乐厅，LQ版可用
	8,		// 山洞
	9,		// 大舞台/竞技场
	10,		// 飞机棚
	11,		// 铺地毯的走廊
	12,		// 一般走廊
	13,		// 石质走廊
	14,		// 胡同/小巷子
	15,		// 森林
	16,		// 城市广场
	17,		// 山脉之中(群山中)
	18,		// 矿场
	19,		// 广阔的平地
	20,		// 停车场
	21,		// 大型水管道
	22,		// 水下
	23,		// 半昏迷状态
	24,		// 眩晕状态
	25,		// 疯狂状态
	26,		// 城堡，小房间
	27,		// 城堡，短通道
	28,		// 城堡，中等房间
	29,		// 城堡，大房间
	30,		// 城堡，长通道
	31,		// 城堡，大厅
	32,		// 城堡，厨房
	33,		// 城堡，庭院
	34,		// 城堡，小亭子
	35,		// 工厂，小房间
	36,		// 工厂，短通道
	37,		// 工厂，中等房间
	38,		// 工厂，大房间
	39,		// 工厂，长通道
	40,		// 工厂，大厅
	41,		// 工厂，厨房
	42,		// 工厂，厂院
	43,		// 工厂，小亭子
	44,		// 冰宫，小房间
	45,		// 冰宫，短通道
	46,		// 冰宫，中等房间
	47,		// 冰宫，大房间
	48,		// 冰宫，长通道
	49,		// 冰宫，大厅
	50,		// 冰宫，厨房
	51,		// 冰宫，花园
	52,		// 冰宫，小亭子
	53,		// 空间站，小房间
	54,		// 空间站，短通道
	55,		// 空间站，中等房间
	56,		// 空间站，大房间
	57,		// 空间站，长通道
	58,		// 空间站，中心厅
	59,		// 空间站，厨房
	60,		// 空间站，植物园
	61,		// 木屋，小房间
	62,		// 木屋，短通道
	63,		// 木屋，中等房间
	64,		// 木屋，大房间
	65,		// 木屋，长通道
	66,		// 木屋，大厅
	67,		// 木屋，厨房
	68,		// 木屋，花圃
	69,		// 木屋，植物园
	70,		// 体育场/馆，空的体育场
	71,		// 体育场/馆，壁球场
	72,		// 体育场/馆，小游泳馆
	73,		// 体育场/馆，大游泳馆
	74,		// 体育场/馆，健身房
	75,		// 体育场/馆，爆满的球场
	76,		// 体育场/馆，体育场的扩音器
	77,		// 工作间
	78,		// 学校大阶梯教室
	79,		// 自习室
	80,		// 小型房车
	81,		// 大型房车
	82,		// 地下圆顶屋
	83,		// 小管道
	84,		// 圆顶圣保罗
	85,		// 细长管道
	86,		// 大管道
	87,		// 会发生共振的管道
	88,		// 后花园
	89,		// 起伏的平原
	90,		// 很深的峡谷
	91,		// 小溪
	92,		// 山谷
	93,		// 心情，愉快、平和
	94,		// 心情，沉重的
	95,		// 心情，回忆中
	96,		// 驾驶/车辆，评论、旁白
	97,		// 驾驶/车辆，停车库
	98,		// 驾驶/车辆，赛车
	99,		// 驾驶/车辆，运动版跑车，轿跑
	100,	// 驾驶/车辆，顶级奢华车
	101,	// 驾驶/车辆，爆满的看台
	102,	// 驾驶/车辆，空的看台
	103,	// 驾驶/车辆，隧道
	104,	// 城市，街道间
	105,	// 城市，地铁
	106,	// 城市，博物馆
	107,	// 城市，图书馆
	108,	// 城市，地下通道
	109,	// 城市，废弃的市区
	110,	// 布满灰尘的房间
	111,	// 教堂，礼拜室
	112		// 小水房
};

/************* 音效库API *************/
// 各效果的参数建议见文档。

// 创建一个音效库实例
// 输出：非NULL成功，NULL失败(内存不足)
void* LINKING CreateVAPInstance();

// 释放一个音效库实例
// 输入参数：音效库实例
void LINKING FreeVAPInstance(void *pInstance);

// 设定PCM数据的采样率
// 输入参数：nSamplingRate[采样率Hz]，pInstance[音效库实例]
// 输出：0成功，-1采样率不支持
int LINKING SetSamplingRate(int nSamplingRate, void *pInstance);

// 设定PCM数据的声道数
// 输入参数：nChannels[通道数量]，pInstance[音效库实例]
// 输出：0成功，-1采样率不支持
int LINKING SetChannels(int nChannels, void *pInstance);

// 设定音效组件的开启或关闭，默认关闭所有组件
// 输入参数：nID[音效组件的ID]，bEnabled[是否启用，0关闭，1启用]，pInstance[音效库实例]
void LINKING EnableComponent(int nID, int bEnabled, void *pInstance);

// 设定伴奏的A/C音量增益控制
// 输入参数：rRatio[控制比率，1.0~5.0]，rVolume[最大音量，0~1.0]，rMaxGain[最大放大倍数，1.0~32.0]，pInstance[音效库实例]
void LINKING SetBGAGCParameters(float rRatio, float rVolume, float rMaxGain, void *pInstance);

// 设定伴奏的3D环绕强度
// 输入参数：rStereoWeight[环绕度，1.0~2.5]，rImageWeight[中央强度，1.0~2.5]，pInstance[音效库实例]
void LINKING SetBGSurParameters(float rStereoWeight, float rImageWeight, void *pInstance);

// 设定伴奏的混响参数
// 输入参数：nPresetID[混响预设配置ID]，rDry[干信号，0~2.0]，rWet[湿信号，0~2.0]，pInstance[音效库实例]
void LINKING SetBGReverbPreset(const int nPresetID, float rDry, float rWet, void *pInstance);

// 设定人声的高频修正和降噪参数
// 输入参数：nBandFreqLimit[高频限制和降噪界限Hz，2000~18000]，pInstance[音效库实例]
void LINKING SetVocalFixParameters(int nBandFreqLimit, void *pInstance);

// 设定人声的谐波处理，LQ版不支持
// 输入参数：fvHarMag[各次谐波的幅度]，pInstance[音效库实例]
void LINKING SetVocalHarmonicParameters(float fvHarMag[10], void *pInstance);

// 设定人声的A/C音量增益控制
// 输入参数：rRatio[控制比率，1.0~5.0]，rVolume[最大音量，0~1.0]，rMaxGain[最大放大倍数，1.0~32.0]，pInstance[音效库实例]
void LINKING SetVocalAGCParameters(float rRatio, float rVolume, float rMaxGain, void *pInstance);

// 设定人声的混响参数
// 输入参数：nPresetID[混响预设配置ID]，rDry[干信号，0~2.0]，rWet[湿信号，0~2.0]，pInstance[音效库实例]
void LINKING SetVocalReverbPreset(const int nPresetID, float rDry, float rWet, void *pInstance);

// 加载一个混响的预设配置到伴奏和人声
// 输入参数：nPresetID[混响预设配置ID]，pInstance[音效库实例]
void LINKING LoadPreset(const int nPresetID, void *pInstance);

// 复位效果器
// 输入参数：pInstance[音效库实例]
void LINKING Reset(void *pInstance);

// 处理伴奏Buffer，16位整数PCM格式
// 输入参数：s16Buffer[PCM数据地址]，nFrameCount[PCM数据帧数]，pInstance[音效库实例]
// 输出参数：处理后，s16Buffer中所包含的PCM数据帧数
int LINKING ProcessBackground_s16(short *s16Buffer, int nFrameCount, void *pInstance);

// 处理伴奏Buffer，32位浮点PCM格式，LQ版不支持
// 输入参数：f32Buffer[PCM数据地址]，nFrameCount[PCM数据帧数]，pInstance[音效库实例]
// 输出参数：处理后，f32Buffer中所包含的PCM数据帧数
int LINKING ProcessBackground_f32(float *f32Buffer, int nFrameCount, void *pInstance);

// 处理人声Buffer，16位整数PCM格式
// 输入参数：s16Buffer[PCM数据地址]，nFrameCount[PCM数据帧数]，pInstance[音效库实例]
// 输出参数：处理后，s16Buffer中所包含的PCM数据帧数
int LINKING ProcessVocal_s16(short *s16Buffer, int nFrameCount, void *pInstance);

// 处理人声Buffer，32位浮点PCM格式，LQ版不支持
// 输入参数：f32Buffer[PCM数据地址]，nFrameCount[PCM数据帧数]，pInstance[音效库实例]
// 输出参数：处理后，f32Buffer中所包含的PCM数据帧数
int LINKING ProcessVocal_f32(float *f32Buffer, int nFrameCount, void *pInstance);
/*************************************/

#ifdef __cplusplus
}
#endif

#endif
