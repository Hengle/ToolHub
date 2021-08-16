using System;

namespace Network
{
    public enum RPCLayer : uint
    {
        Layer0 = 0x1u,
        Layer1 = 0x2u,
        Layer2 = 0x4u,
        Layer3 = 0x8u,
        Layer4 = 0x10u,
        Layer5 = 0x20u,
        Layer6 = 0x40u,
        Layer7 = 0x80u,
        Layer8 = 0x100u,
        Layer9 = 0x200u,
        Layer10 = 0x400u,
        Layer11 = 0x800u,
        Layer12 = 0x1000u,
        Layer13 = 0x2000u,
        Layer14 = 0x4000u,
        Layer15 = 0x8000u,
        Layer16 = 0x10000u,
        Layer17 = 0x20000u,
        Layer18 = 0x40000u,
        Layer19 = 0x80000u,
        Layer20 = 0x100000u,
        Layer21 = 0x200000u,
        Layer22 = 0x400000u,
        Layer23 = 0x800000u,
        Layer24 = 0x1000000u,
        Layer25 = 0x2000000u,
        Layer26 = 0x4000000u,
        Layer27 = 0x8000000u,
        Layer28 = 0x10000000u,
        Layer29 = 0x20000000u,
        Layer30 = 0x40000000u,
        Layer31 = 0x80000000u,
        None = 0x0u,
        All = 0xffffffff
    };
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct, AllowMultiple = false, Inherited = false)]

    public class RPCAttribute : Attribute
    {
        public RPCLayer layer;
        public RPCAttribute(RPCLayer layer)
        {
            this.layer = layer;
        }
    }
}