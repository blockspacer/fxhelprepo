#pragma once

#ifdef _WIN64
	#pragma comment(lib, "VMProtectSDK64.lib")
#else
	#pragma comment(lib, "VMProtectSDK32.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

// protection
// 功能:设置开始标记
__declspec(dllimport) void __stdcall VMProtectBegin(const char *);
// 开始虚拟化代码处标记（包括保护设置）
__declspec(dllimport) void __stdcall VMProtectBeginVirtualization(const char *);
// 开始变异代码处标记（包括保护设置）
__declspec(dllimport) void __stdcall VMProtectBeginMutation(const char *);
// 开始虚拟+代码变异标记处
__declspec(dllimport) void __stdcall VMProtectBeginUltra(const char *);
// 绑定代码到序列号,进行虚拟保护
__declspec(dllimport) void __stdcall VMProtectBeginVirtualizationLockByKey(const char *);
// 绑定代码到序列号, 进行虚拟 + 变异保护
__declspec(dllimport) void __stdcall VMProtectBeginUltraLockByKey(const char *);
// 设置与虚拟/变异等功能配对的结束标记
__declspec(dllimport) void __stdcall VMProtectEnd(void);

// utils
// 检测调试器
__declspec(dllimport) BOOL __stdcall VMProtectIsDebuggerPresent(BOOL);
// 检测虚拟机
__declspec(dllimport) BOOL __stdcall VMProtectIsVirtualMachinePresent(void);
// 映像文件CRC校验
__declspec(dllimport) BOOL __stdcall VMProtectIsValidImageCRC(void);
// 功能:加密Ansi字符串常量, 返回加密后的字符串指针，主要是防止字符串被搜索到
__declspec(dllimport) char * __stdcall VMProtectDecryptStringA(const char *value);
// 功能:加Unicode字符串,返回加密后的字符串指针
__declspec(dllimport) wchar_t * __stdcall VMProtectDecryptStringW(const wchar_t *value);
// 释放所有被加密的字符串
__declspec(dllimport) BOOL __stdcall VMProtectFreeString(void *value);

// licensing
enum VMProtectSerialStateFlags
{
	SERIAL_STATE_FLAG_CORRUPTED			= 0x00000001,
	SERIAL_STATE_FLAG_INVALID			= 0x00000002,
	SERIAL_STATE_FLAG_BLACKLISTED		= 0x00000004,
	SERIAL_STATE_FLAG_DATE_EXPIRED		= 0x00000008,
	SERIAL_STATE_FLAG_RUNNING_TIME_OVER	= 0x00000010,
	SERIAL_STATE_FLAG_BAD_HWID			= 0x00000020,
	SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED	= 0x00000040,
};

#pragma pack(push, 1)

typedef struct
{
	WORD			wYear;
	BYTE			bMonth;
	BYTE			bDay;
} VMProtectDate;

typedef struct
{
	INT				nState;				// VMProtectSerialStateFlags 需要设置的注册信息
	wchar_t			wUserName[256];		// 用户名
	wchar_t			wEMail[256];		// 邮箱
	VMProtectDate	dtExpire;			// 截止日期
	VMProtectDate	dtMaxBuild;			// 授权日期
	INT				bRunningTime;		// 运行时间
	BYTE			nUserDataLength;	// 使用者信息长度
	BYTE			bUserData[255];		// 使用者信息
} VMProtectSerialNumberData;

#pragma pack(pop)

// 设置序列号, 将序列号传递给授权模块, 返回零成功
__declspec(dllimport) INT  __stdcall VMProtectSetSerialNumber(const char * SerialNumber);
// 返回序列号状态,由常量定义的位组成,正常返回0,如果为常量定义的任意一位或多个值,则序列号为非法序列号
__declspec(dllimport) INT  __stdcall VMProtectGetSerialNumberState();
// 如果授权模块错误或者是VMProtectSerialNumberData结构或者其大小异常, 函数都将返回False
__declspec(dllimport) BOOL __stdcall VMProtectGetSerialNumberData(VMProtectSerialNumberData *pData, UINT nSize);
// 获取当前电脑的硬件识别码.如果HWID参数为NULL,nSize为0,则返回储存硬件识别码所需要的空间大小
__declspec(dllimport) INT  __stdcall VMProtectGetCurrentHWID(char * HWID, UINT nSize);

// activation
enum VMProtectActivationFlags
{
	ACTIVATION_OK = 0,
	ACTIVATION_SMALL_BUFFER,
	ACTIVATION_NO_CONNECTION,
	ACTIVATION_BAD_REPLY,
	ACTIVATION_BANNED,
	ACTIVATION_CORRUPTED,
	ACTIVATION_BAD_CODE,
	ACTIVATION_ALREADY_USED,
	ACTIVATION_SERIAL_UNKNOWN,
};
// 激活许可证
__declspec(dllimport) INT __stdcall VMProtectActivateLicense(const char *code, char *serial, int size);
// 停用许可证
__declspec(dllimport) INT __stdcall VMProtectDeactivateLicense(const char *serial);
// 获得离线激活字符串
__declspec(dllimport) INT __stdcall VMProtectGetOfflineActivationString(const char *code, char *buf, int size);
// 得到离线反激活字符串
__declspec(dllimport) INT __stdcall VMProtectGetOfflineDeactivationString(const char *serial, char *buf, int size);


#ifdef __cplusplus
}
#endif
