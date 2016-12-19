#include "stdafx.h"
#include <memory>
#include <math.h>
#include "MathHelper.h"

namespace
{
    template<typename T>
    int partition(T *A, int begin, int end)
    {
        int i = begin;
        int j = end;
        T mid = A[begin];
        while (i < j)
        {
            while (A[j] >= mid && i < j)
            {
                --j;
            }
            if (i < j)
            {
                A[i] = A[j];
            }

            while (A[i] <= mid && i < j)
            {
                ++i;
            }
            if (i < j)
            {
                A[j] = A[i];
            }
        }
        A[j] = mid;
        return j;
    }
    // 求一个长为m的数组中第k(k<m)大的数值  
    // 输入: 数组指针A，数组长度len，k值  
    // 返回: 第k+1小的数  
    template<typename T>
    T PartitionSortGetMinK(T *A, int begin, int end, int k)
    {
        T mid = partition<T>(A, begin, end);
        if (k == mid)
        {
            return A[mid];
        }
        else if (k < mid)
        {
            return PartitionSortGetMinK(A, begin, mid - 1, k);
        }
        return PartitionSortGetMinK(A, mid + 1, end, k);
    }
}
MathHelper::MathHelper()
{
}


MathHelper::~MathHelper()
{
}

uint32 MathHelper::GetSum(const std::vector<uint32>& data)
{
    return 0;
}

double MathHelper::GetAvg(const std::vector<uint32>& data)
{
    return 0;
}

uint32 MathHelper::GetMid(const std::vector<uint32>& data)
{
    std::unique_ptr<uint32> ptr(new uint32[data.size()]);
    for (auto i = 0; i < data.size();++i)
    {
        (ptr.get())[i] = data[i];
    }
    
    return PartitionSortGetMinK(ptr.get(), 0, data.size(), data.size() / 2);
}

double MathHelper::GetStandardDeviation(const std::vector<uint32>& data, double avg)
{
    double variance = 0.0;
    for (auto it : data)
    {
        double t = (it - avg);
        variance += t*t;
    }

    if (variance<=0.0)
    {
        return 0.0;
    }
    return sqrt(variance)/data.size();
}
