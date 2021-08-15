using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Runtime.CompilerServices;
using System;
namespace Native
{
    public unsafe struct NativeListData
    {
        public long count;
        public long capacity;
        public void* ptr;
    }
    public unsafe struct NativeList<T> : IEnumerable<T> where T : unmanaged
    {
        private NativeListData* data;
        public bool isCreated { get; private set; }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public NativeList(long capacity, long count = 0)
        {
            isCreated = true;
            capacity = Math.Max(capacity, 1);
            data = (NativeListData*)Memory.vengine_malloc((ulong)sizeof(NativeListData));
            data->count = 0;
            data->capacity = capacity;
            data->ptr = (T*)Memory.vengine_malloc((ulong)sizeof(T) * (ulong)capacity);
        }
        private void Resize()
        {
            if (data->count <= data->capacity) return;
            data->capacity = Math.Max(data->capacity + 1, (long)(data->capacity * 1.5));
            void* newPtr = (T*)Memory.vengine_malloc((ulong)sizeof(T) * (ulong)data->capacity);
            Memory.vengine_memcpy(newPtr, data->ptr, (ulong)sizeof(T) * (ulong)data->count);
            Memory.vengine_free(data->ptr);
            data->ptr = newPtr;
        }

        private void ResizeToCount()
        {
            if (data->count <= data->capacity) return;
            long oldCap = data->capacity;
            data->capacity = Math.Max((long)(oldCap * 1.2f), data->count);
            void* newPtr = (T*)Memory.vengine_malloc((ulong)sizeof(T) * (ulong)data->capacity);
            Memory.vengine_memcpy(newPtr, data->ptr, (ulong)sizeof(T) * (ulong)oldCap);
            Memory.vengine_free(data->ptr);
            data->ptr = newPtr;
        }

        public void AddCapacityTo(in long capacity)
        {
            if (capacity <= data->capacity) return;
            data->capacity = capacity;
            void* newPtr = (T*)Memory.vengine_malloc((ulong)sizeof(T) * (ulong)data->capacity);
            Memory.vengine_memcpy(newPtr, data->ptr, (ulong)sizeof(T) * (ulong)data->count);
            Memory.vengine_free(data->ptr);
            data->ptr = newPtr;
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void RemoveLast(in long length)
        {
            data->count -= length;
            data->count = Math.Max(0, data->count);
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void RemoveLast()
        {
            data->count = Math.Max(0, data->count - 1);
        }
        public void RemoveAt(in long index)
        {
            long last = Length - 1;
            for (long i = index; i < last; ++i)
            {
                this[i] = this[i + 1];
            }
            data->count--;
        }


        public void RemoveElement(in T target, in System.Func<T, T, bool> conditionFunc)
        {
            for (long i = 0; i < Length; ++i)
            {
                while (conditionFunc(target, this[i]) && i < Length)
                {
                    this[i] = this[Length - 1];
                    RemoveLast();
                }
            }
        }

        public long Length
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                return data->count;
            }
        }
        public long Capacity
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                return data->capacity;
            }
        }
        public T* unsafePtr
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                return (T*)data->ptr;
            }
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Dispose()
        {
            if (isCreated)
            {
                isCreated = false;
                Memory.vengine_free(data->ptr);
                Memory.vengine_free(data);
            }
        }
        public ref T this[in long id]
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                T* ptr = (T*)data->ptr;
                return ref *(ptr + id);
            }
        }

        public ref T this[in int id]
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                T* ptr = (T*)data->ptr;
                return ref *(ptr + id);
            }
        }
        public ref T this[in uint id]
        {
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                T* ptr = (T*)data->ptr;
                return ref *(ptr + id);
            }
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void AddRange(in long length)
        {
            data->count += length;
            ResizeToCount();
        }
        public void AddRange(in T[] array)
        {
            long last = data->count;
            data->count += (long)array.Length;
            ResizeToCount();
            fixed (void* source = &array[0])
            {
                void* dest = unsafePtr + last;
                Memory.vengine_memcpy(dest, source, (ulong)array.Length * (ulong)sizeof(T));
            }
        }

        public void AddRange(in T* array, in long length)
        {
            long last = data->count;
            data->count += length;
            ResizeToCount();
            void* dest = unsafePtr + last;
            Memory.vengine_memcpy(dest, array, (ulong)length * (ulong)sizeof(T));

        }
        public void AddRange(in NativeList<T> array)
        {
            long last = data->count;
            data->count += array.Length;
            ResizeToCount();
            void* dest = unsafePtr + last;
            Memory.vengine_memcpy(dest, array.unsafePtr, (ulong)array.Length * (ulong)sizeof(T));
        }
        public long ConcurrentAdd(in T value)
        {
            long last = Interlocked.Increment(ref data->count);
            //Concurrent Resize
            if (last <= data->capacity)
            {
                last--;
                T* ptr = (T*)data->ptr;
                *(ptr + last) = value;
                return last;
            }
            Interlocked.Exchange(ref data->count, data->capacity);
            return long.MaxValue;
        }
        public long ConcurrentAdd(in T value, in object lockerObj)
        {
            long last = Interlocked.Increment(ref data->count);
            //Concurrent Resize
            if (last > data->capacity)
            {
                lock (lockerObj)
                {
                    if (last > data->capacity)
                    {
                        long newCapacity = data->capacity * 2;
                        void* newPtr = Memory.vengine_malloc((ulong)sizeof(T) * (ulong)newCapacity);
                        Memory.vengine_memcpy(newPtr, data->ptr, (ulong)sizeof(T) * (ulong)data->count);
                        Memory.vengine_free(data->ptr);
                        data->ptr = newPtr;
                        data->capacity = newCapacity;
                    }
                }
            }
            last--;
            T* ptr = (T*)data->ptr;
            *(ptr + last) = value;
            return last;
        }
        public void Add(in T value)
        {
            long last = data->count;
            data->count++;
            Resize();
            T* ptr = (T*)data->ptr;
            *(ptr + last) = value;
        }

        public void Insert(in T value, in long index)
        {
            data->count++;
            Resize();
            T* ptr = (T*)data->ptr;
            for (long i = data->count - 1; i > index; --i)
            {
                ptr[i] = ptr[i - 1];
            }
            ptr[index] = value;
        }

        public void Remove(in long i)
        {
            ref long count = ref data->count;
            if (count == 0) return;
            count--;
            this[i] = this[count];
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Clear()
        {
            data->count = 0;
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public IEnumerator<T> GetEnumerator()
        {
            return new ListIenumerator<T>(data);
        }
    }

    public unsafe struct ListIenumerator<T> : IEnumerator<T> where T : unmanaged
    {
        private NativeListData* data;
        private long iteIndex;
        public ListIenumerator(NativeListData* dataPtr)
        {
            data = dataPtr;
            iteIndex = long.MaxValue;
        }
        object IEnumerator.Current
        {
            get
            {
                return ((T*)data->ptr)[iteIndex];
            }
        }

        public T Current
        {
            get
            {
                return ((T*)data->ptr)[iteIndex];
            }
        }

        public bool MoveNext()
        {
            return (++iteIndex < (data->count));
        }

        public void Reset()
        {
            iteIndex = long.MaxValue;
        }

        public void Dispose()
        {
        }
    }
}