using System;
using System.Collections.Generic;
using Native;
public unsafe class ArenaAllocator : IDisposable
{
    List<IntPtr> allocatedPtr = null;
    struct Pool
    {
        public ulong ptr;
        public ulong leftSize;
    }
    List<Pool> allocList;

    ulong minSize = 1;
    public void Dispose()
    {
        if (allocatedPtr == null) return;
        Clear();
        allocatedPtr = null;
    }
    public void Clear()
    {
        lock (allocatedPtr)
        {
            foreach (var i in allocatedPtr)
            {
                Memory.vengine_free(i.ToPointer());
            }
            allocatedPtr.Clear();
            allocList.Clear();
        }
    }
    const ulong ALIGN = 16;
    static ulong CalcConstantBufferByteSize(ulong byteSize)
    {
        return (byteSize + (ALIGN - 1)) & ~(ALIGN - 1);
    }

    public ArenaAllocator(ulong sz = 65536)
    {
        sz = CalcConstantBufferByteSize(sz);
        allocatedPtr = new List<IntPtr>();
        allocList = new List<Pool>();
        minSize = sz;
        Allocate(0);
    }
    ulong vv = 0;
    void* Allocate(ulong takedSize)
    {
        void* ptr = Memory.vengine_malloc(minSize);
        vv += minSize;
        Console.WriteLine("Allocated Size: " + vv);
        allocatedPtr.Add(new IntPtr(ptr));
        allocList.Add(new Pool { ptr = (ulong)(ptr) + takedSize, leftSize = minSize - takedSize });
        return ptr;
    }
    public void* Malloc(ulong size)
    {
        size = CalcConstantBufferByteSize(size);
        lock (allocatedPtr)
        {
            if (size >= minSize)
            {
                void* newPtr = Memory.vengine_malloc(size);
                allocatedPtr.Add(new IntPtr(newPtr));
                return newPtr;
            }
            for (int i = 0; i < allocList.Count; ++i)
            {
                var v = allocList[i];
                if (v.leftSize >= size)
                {
                    void* oriPtr = (void*)v.ptr;
                    v.leftSize -= size;
                    v.ptr += size;
                    if ((v.leftSize - size) < ALIGN)
                    {
                        allocList[i] = allocList[allocList.Count - 1];
                        allocList.RemoveAt(allocList.Count - 1);
                        --i;
                    }
                    else
                    {
                        allocList[i] = v;
                    }
                    return oriPtr;
                }
            }
            minSize *= 2;
            return Allocate(size);
        }
    }
}

