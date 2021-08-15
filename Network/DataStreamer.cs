using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Native;

namespace Network
{
    public unsafe interface IStreamerVisitor
    {
        public void GetNextByteArray(out byte[] byteArr, out ulong usedByteLen);
        public Task Execute(IntPtr data, ulong byteLength);
    }
    public class DataStreamer
    {
        byte[] currentArr = null;
        ulong leftedBytes = 0;
        ulong arrUsedBytes = 0;
        unsafe void GetNext(IStreamerVisitor visitor, byte* dest, ulong targetSize)
        {
            while (targetSize > 0)
            {
                //array have more data than require
                if (leftedBytes >= targetSize)
                {
                    fixed (byte* src = currentArr)
                    {
                        Memory.vengine_memcpy(dest, src + arrUsedBytes - leftedBytes, targetSize);
                    }
                    leftedBytes -= targetSize;
                    var bb = currentArr;
                    var vv = leftedBytes;
                    if (leftedBytes == 0)
                        currentArr = null;
                    break;
                }
                else
                {
                    if (leftedBytes > 0)
                    {
                        fixed (byte* src = currentArr)
                        {
                            Memory.vengine_memcpy(dest, src + arrUsedBytes - leftedBytes, leftedBytes);
                        }
                        dest += leftedBytes;
                        targetSize -= leftedBytes;
                    }
                    visitor.GetNextByteArray(out currentArr, out arrUsedBytes);
                    leftedBytes = arrUsedBytes;
                }
            }
        }

        async void RunTask(IStreamerVisitor visitor, IntPtr data, ulong byteLength)
        {
            unsafe static void Free(IntPtr ptr)
            {
                Memory.vengine_free(ptr.ToPointer());

            }
            await visitor.Execute(data, byteLength);
            await Task.Run(() =>
            {
                Free(data);
            });
        }


        public unsafe void StreamNext(IStreamerVisitor visitor, ulong targetSize)
        {
            byte* ptr = (byte*)Memory.vengine_malloc(targetSize);
            GetNext(visitor, ptr, targetSize);
            RunTask(visitor, new IntPtr(ptr), targetSize);
        }
        public unsafe void StreamNext_InterSize(IStreamerVisitor visitor)
        {
            ulong targetSize = 0;
            GetNext(visitor, (byte*)&targetSize, sizeof(ulong));
            byte* ptr = (byte*)Memory.vengine_malloc(targetSize + sizeof(ulong));
            *(ulong*)ptr = targetSize;
            GetNext(visitor, ptr + sizeof(ulong), targetSize);
            RunTask(visitor, new IntPtr(ptr), targetSize + sizeof(ulong));
        }
    }
}