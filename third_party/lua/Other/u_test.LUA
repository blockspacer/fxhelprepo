--TEST_PATH = "\\Storage Card\\Test\\"
--TEST_DRIVER = "\\Storage Card\\"
--test.SDInitCounter("SDH1:"); SDout=true --sd信息输出, 需要更改配置
--TEST_PATH = "\\FLASH2\\Test\\"
--TEST_DRIVER = "\\FLASH2\\"

TEST_PATH = "G:\\Test\\"
TEST_DRIVER = "G:"

TEST_COPY_PATH = "\\"
SD_PATH = "\\Storage Card\\"

USE_HOOK = false                -- 使用Hook, 在发现目标目录不在时, 停止往下跑. 可以在异常测试中使用. 

GPIO_TEST = false
GPIO_WAITTIME = 10000
GPIO_LOCKTIME = 5000

gpio_table = {
24, 25, 26, 27,                 --SDIO, share with UART4, skip if COM4 is load
28, 29,                         --SDIO
30, 31, 32, 33, 34, 35, 36, 37, --NandFlash / SDHC
41, 42, 43, 44, 45, 46,         --NandFlash
39, 40,                         --SDHC
38,                             --NandFlash, CE1
72, 73, 74, 75                  --SDHC, ak7801 only
}



QUICK_FILL = true
------------=======标准配置=======------------
FILE_BEGIN          = 0
FILE_CURRENT        = 1
FILE_END            = 2
collectgarbage("setpause", 100)

------------=======Hook函数=======------------
function trace (event)
	--local s = debug.getinfo(2).name
	if test.FileExists(TEST_DRIVER) == false or test.GetFATInfo(TEST_DRIVER) == false then
           print("The Driver is no longer ready for test")
           os.exit()
	end
end

if USE_HOOK then
    debug.sethook(trace, "c")
end
------------=======公共函数=======------------
ResultTable = {}
ResultPointer = 1

function PrintStat()
    test.Date();
    test.GetInfo()
    test.MemWatch()
    test.GetFATInfo(TEST_DRIVER)
end

function PrintStart(CaseID, rst)
    test.LogOut("========================================")
    test.LogOut(TEST_PATH .. "Start CaseID "..CaseID..":"..rst)
    test.LogOut("========================================")
    PrintStat()
end

function PrintResult(CaseID, rst)
    out = ""
    PrintStat()
    test.LogOut("========================================")
    if rst then
        out = "TRUE"
    else
        out = "FALSE"
    end
    test.LogOut(TEST_PATH .. "CaseID "..CaseID.." Result>:"..out)
    test.LogOut("========================================")

    ResultTable[ResultPointer] = TEST_PATH .. "CaseID "..CaseID.." Result>:"..out
    ResultPointer = ResultPointer+1
end


if test.FileExists(TEST_PATH) then
    test.DeleteTree(TEST_PATH, false)
end



function Test_1()
    PrintStart("FAL_0001, FAL_0002", "Create\\Write\\Read\\Delete a file, List a dir")

    CaseFileLen = {1, 100, 511, 512, 513, 2047, 2048, 2049, 4095, 4096, 4097}
    CaseRWLen ={1, 100, 511, 512, 513, 2047, 2048, 2049, 4095, 4096, 4097}
    Buffer={0x00,0x5a,0x90,0xff}
    TestFileTmpName = "Test_L"
    Result = true

    Result = Result and test.CreateDirectory(TEST_PATH)

    for i, filelen in ipairs(CaseFileLen) do
        for j, rwlen in ipairs(CaseRWLen) do
            if filelen >= rwlen then
                TestFileName = TestFileTmpName..filelen.."_"..rwlen
                Result = Result and test.CreateFile(TEST_PATH, TestFileName, Buffer, rwlen, filelen)
                Result = Result and test.VerifFile(TEST_PATH, TestFileName, Buffer, rwlen, filelen)
                if not Result then
                    break
                end
            end
        end
        if not Result then
            break
        end
    end

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0001", Result)
end


function Test_2()
    PrintStart("FAL_0003", "Copy a file.")
    Buffer={0x00,0x5a,0x90,0xff}
    Result = true
    TestFileName = "TEST_COPY"
    TestCopyName = "TEST_COPY2"
    TestFileSize = 10*1024*1024

    Result = Result and test.CreateDirectory(TEST_PATH)

    Result = Result and test.CreateFile(TEST_PATH, TestFileName, Buffer, 0, TestFileSize)
    Result = Result and test.CopyFile(TEST_PATH..TestFileName, TEST_PATH..TestCopyName, true)
    Result = Result and test.CompareFiles(TEST_PATH..TestFileName, TEST_PATH..TestCopyName)
    Result = Result and test.DeleteFile(TEST_PATH..TestFileName)
    Result = Result and test.DeleteFile(TEST_PATH..TestCopyName)

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0003", Result)
end


function Test_3()
    PrintStart("FAL_0004", "Copy many files.")
    Buffer={0x00,0x5a,0x90,0xff}
    TestSrcDirName = "Src"
    TestDstDirName = "Dst"
    TestFileTmpName = "File."
    TestFileCnt = 100
    TestFileSizeInc = 1024
    Result = true

    Result = Result and test.CreateDirectory(TEST_PATH)
    Result = Result and test.CreateDirectory(TEST_PATH.."\\"..TestSrcDirName)
    Result = Result and test.CreateDirectory(TEST_PATH.."\\"..TestDstDirName)


    for i=1, TestFileCnt do
        Result = Result and test.CreateFile(TEST_PATH..TestSrcDirName, TestFileTmpName..i, Buffer, 0, i*TestFileSizeInc)
    end

    Result = Result and test.CopyDirectory(TEST_PATH..TestSrcDirName, TEST_PATH..TestDstDirName, true);
    Result = Result and test.CompareDirectory(TEST_PATH..TestSrcDirName, TEST_PATH..TestDstDirName)

    Result = test.DeleteTree(TEST_PATH.."\\"..TestSrcDirName, false) and Result
    Result = test.DeleteTree(TEST_PATH.."\\"..TestDstDirName, false) and Result
    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0004", Result)
end


function Test_4()
    PrintStart("FAL_0005", "Move a file.")
    Buffer={0x00,0x5a,0x90,0xff}
    TestSrcFileName = "TEST_MOVE"
    TestDstFileName = "TEST_MOVE2"
    TestFileSize = 10*1024*1024
    Result = true
    Result = Result and test.CreateDirectory(TEST_PATH)

    Result = Result and test.CreateFile(TEST_PATH, TestSrcFileName, Buffer, 0, TestFileSize)
    Result = Result and test.MoveFile(TEST_PATH..TestSrcFileName, TEST_PATH..TestDstFileName)
    Result = Result and test.VerifFile(TEST_PATH, TestDstFileName, Buffer, 0, TestFileSize)
    Result = Result and test.DeleteFile(TEST_PATH..TestDstFileName)

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0005", Result)
end

function Test_5()
    PrintStart("FAL_0006", "Move many files.")
    Buffer={0x00,0x5a,0x90,0xff}
    TestSrcDirName = "Src"
    TestDstDirName = "Dst"
    TestFileTmpName = "File."
    TestFileCnt = 100
    TestFileSizeInc = 1024
    Result = true

    Result = Result and test.CreateDirectory(TEST_PATH)
    Result = Result and test.CreateDirectory(TEST_PATH.."\\"..TestSrcDirName)
    Result = Result and test.CreateDirectory(TEST_PATH.."\\"..TestDstDirName)

    for i=1, TestFileCnt do
        Result = Result and test.CreateFile(TEST_PATH..TestSrcDirName, TestFileTmpName..i, Buffer, 0, i*TestFileSizeInc)
    end

    Result = Result and test.MoveDirectory(TEST_PATH..TestSrcDirName, TEST_PATH..TestDstDirName, true);
    Result = Result and test.DeleteTree(TEST_PATH..TestDstDirName, false)

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0006", Result)
end

function Test_6(case_id, FirstDriverPath, SecondDriverPath)
    PrintStart(case_id, "Copy, Move a file between 2 drivers.")
    Buffer={0x00,0x5a,0x90,0xff}
    FileSize = 100*1024
    TestFileName = "TEST_COPY"
    TestFileNewName = "TEST_COPY2"

    Result = true

    Result = Result and test.CreateDirectory(TEST_PATH)

    Result = Result and test.CreateFile(FirstDriverPath, TestFileName, Buffer, 0, FileSize)
    Result = Result and test.CopyFile(FirstDriverPath..TestFileName, SecondDriverPath..TestFileName, true)
    Result = Result and test.MoveFile(SecondDriverPath..TestFileName, FirstDriverPath..TestFileNewName, true)
    Result = Result and test.CompareFiles(FirstDriverPath..TestFileName, FirstDriverPath..TestFileNewName)

    Result = Result and test.DeleteFile(FirstDriverPath..TestFileName)
    Result = Result and test.DeleteFile(FirstDriverPath..TestFileNewName)

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult(case_id, Result)
end

function Test_7()
    PrintStart("FAL_0012", "Cut a file.")
    Buffer={0x00,0x5a,0x90,0xff}
    TestFileName = "TEST_CUT"
    TestFileSize = 100000
    TestCutSize = -10000

    Result = true

    Result = Result and test.CreateDirectory(TEST_PATH)
    Result = Result and  test.CreateFile(TEST_PATH, TestFileName, Buffer, 0, TestFileSize)

    Result = Result and  test.CutFile(TEST_PATH..TestFileName, TestCutSize, FILE_END)

    if TestFileSize+TestCutSize ~= test.FileSize(TEST_PATH..TestFileName) then
        test.LogOut("File Size Error!") 
        Result = false
    end

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0012", Result)
end 

function Test_8()
    PrintStart("FAL_0013", "Seek Test.")
    Buffer={0x00,0x5a,0x90,0xff}
    SeekBuffer = {0xff,0x90,0x5a,0x00}

    SeekPos = {0,  511, 512, 513, 1023, 1024, 1025, 2047, 2048, 2049, 4095, 4096, 4097}
    fSeekPos = {0-4096,  -511-4096, -512-4096, -513-4096, -1023-4096, -1024-4096, -1025-4096, -2047-4096, -2048-4096, -2049-4096, -4095-4096, -4096-4096, -4097-4096}
    OperatorLen = {2, 512, 2048, 4096}

    TestFileName = "TEST_SEEK"
    TestFileSize = 100*1024
    Result = true


    Result = Result and test.CreateDirectory(TEST_PATH)

    Result = Result and test.CreateFile(TEST_PATH, TestFileName, Buffer, 0, TestFileSize)
    Result = Result and test.SeekTest(TEST_PATH..TestFileName, SeekPos, OperatorLen, SeekBuffer, FILE_BEGIN)
    Result = Result and test.SeekTest(TEST_PATH..TestFileName,fSeekPos, OperatorLen,SeekBuffer, FILE_END)

    Result = test.DeleteTree(TEST_PATH, false) and Result
    PrintResult("FAL_0013", Result)
end

if GPIO_TEST then
    test.GpioTest(gpio_table, GPIO_WAITTIME, GPIO_LOCKTIME)
end

Test_1()
Test_2()
Test_3()
Test_4()
Test_5()
Test_6("FAL_0007",TEST_PATH, TEST_COPY_PATH)
Test_6("FAL_0008",TEST_COPY_PATH, TEST_PATH)

if test.FileExists(SD_PATH) then
    Test_6("FAL_0009",TEST_PATH, SD_PATH)
else
    PrintResult("FAL_0009_SKIP(NO SDCard)", false)
end

Test_7()
Test_8()

if SDout then
    test.SDPrintCountRst("SDH1:")
end

if GPIO_TEST then
    test.GpioStop(2000)
end
--==================print all==============--
for i, rst in ipairs(ResultTable) do
    test.LogOut(rst)
end