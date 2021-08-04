using System;
public interface IFunction<R>
{
    R Run();
}

public interface IFunction<A, R>
{
    R Run(ref A a);
}

public interface IFunction<A, B, R>
{
    R Run(ref A a, ref B b);
}

public interface IFunction<A, B, C, R>
{
    R Run(ref A a, ref B b, ref C c);
}

public interface IFunction<A, B, C, D, R>
{
    R Run(ref A a, ref B b, ref C c, ref D d);
}

public interface IFunction<A, B, C, D, E, R>
{
    R Run(ref A a, ref B b, ref C c, ref D d, ref E e);
}

public interface IFunction<A, B, C, D, E, F, R>
{
    R Run(ref A a, ref B b, ref C c, ref D d, ref E e, ref F f);
}

public interface IFunction<A, B, C, D, E, F, G, R>
{
    R Run(ref A a, ref B b, ref C c, ref D d, ref E e, ref F f, ref G g);
}

public interface IFunction<A, B, C, D, E, F, G, H, R>
{
    R Run(ref A a, ref B b, ref C c, ref D d, ref E e, ref F f, ref G g, ref H h);
}
public struct IntEqual : IFunction<int, int, bool>
{
    public bool Run(ref int a, ref int b)
    {
        return a == b;
    }
}
public struct UintEqual : IFunction<uint, uint, bool>
{
    public bool Run(ref uint a, ref uint b)
    {
        return a == b;
    }
}
public struct LongEqual : IFunction<long, long, bool>
{
    public bool Run(ref long a, ref long b)
    {
        return a == b;
    }
}
public struct UlongEqual : IFunction<ulong, ulong, bool>
{
    public bool Run(ref ulong a, ref ulong b)
    {
        return a == b;
    }
}
public struct CharEqual : IFunction<char, char, bool>
{
    public bool Run(ref char a, ref char b)
    {
        return a == b;
    }
}
public struct UshortEqual : IFunction<ushort, ushort, bool>
{
    public bool Run(ref ushort a, ref ushort b)
    {
        return a == b;
    }
}
public struct ShortEqual : IFunction<short, short, bool>
{
    public bool Run(ref short a, ref short b)
    {
        return a == b;
    }
}

[Serializable]
public struct Pair<T, V>
{
    public T key;
    public V value;
    public Pair(T key, V value)
    {
        this.key = key;
        this.value = value;
    }
}

