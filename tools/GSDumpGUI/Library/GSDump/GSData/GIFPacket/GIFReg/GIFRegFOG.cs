﻿using System;
using System.Collections.Generic;
using System.Text;

namespace GSDumpGUI
{
    public class GIFRegFOG : GIFReg
    {
        public double F;

        static public GIFReg Unpack(GIFTag tag, int addr, UInt64 LowData, UInt64 HighData, bool PackedFormat)
        {
            GIFRegFOG u = new GIFRegFOG();
            u.Descriptor = (GIFRegDescriptor)addr;
            if (PackedFormat)
                u.F = (UInt16)(GetBit(HighData, 36, 8));
            else
                u.F = GetBit(LowData, 56, 8);
            return u;
        }
    }
}
