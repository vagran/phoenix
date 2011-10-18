/*
 * /phoenix/kernel/sys/arch/x86_64/md_cpu_caps.h
 *
 * This file is a part of Phoenix operating system.
 * Copyright (c) 2011, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file md_cpu_caps.h
 * Machine-dependent CPU capabilities enquiring. Should be usable from both
 * the kernel and user space.
 */

#ifndef MD_CPU_CAPS_H_
#define MD_CPU_CAPS_H_

#ifdef AUTONOMOUS_LINKING
namespace {
#endif /* AUTONOMOUS_LINKING */

namespace cpu {

/** This class is used for CPU capabilities enquiring. */
class CpuCaps {
public:
    /** Retrieve value of the specified capability for the current CPU.
     * @param cap The capability to retrieve. If @a CPU_CAP_NONE specified
     *      then this method returns maximal support capability ID.
     * @return The requested capability value. The value is interpreted
     *      specifically for each particular capability. Zero is returned in
     *      case the capability is not known.
     */
    unsigned long GetCapability(CpuCapId cap) {
        if (cap >= CPU_CAP_MAX) {
            return 0;
        }
        if (cap == CPU_CAP_NONE) {
            return CPU_CAP_MAX;
        }

        FeatureDesc feature;
        if (!MapCapability(cap, &feature)) {
            return 0;
        }

        u32 maxFeature, maxExtFeature;
        cpuid(0, 0, &maxFeature, 0, 0, 0);
        cpuid(EXT_FEAT_BASE, 0, &maxExtFeature, 0, 0, 0);

        if ((feature.cpuidFeatureId < EXT_FEAT_BASE && feature.cpuidFeatureId > maxFeature) ||
            (feature.cpuidFeatureId >= EXT_FEAT_BASE && feature.cpuidFeatureId > maxExtFeature)) {
            /* The feature is not present in this CPU. */
            return feature.defValue;
        }

        u32 regs[RES_MAX];
        cpuid(feature.cpuidFeatureId, feature.cpuidFeatureSubid,
              &regs[RES_EAX], &regs[RES_EBX], &regs[RES_ECX], &regs[RES_EDX]);

        return (regs[feature.resultReg] >> feature.bitIdx) &
               ((1 << feature.numBits) - 1);
    }

private:
    enum CpuidResultReg {
        RES_EAX,
        RES_EBX,
        RES_ECX,
        RES_EDX,
        RES_MAX
    };

    enum {
        EXT_FEAT_BASE = 0x80000000
    };

    struct FeatureDesc {
        CpuCapId cap;
        long cpuidFeatureId, cpuidFeatureSubid;
        CpuidResultReg resultReg;
        int bitIdx, numBits;
        unsigned long defValue;
    };

    bool MapCapability(CpuCapId cap, FeatureDesc *feat) {
        static FeatureDesc features[] = {
            { CPU_CAP_PG_PGE, 0x1, 0, RES_EDX, 13, 1, 0 },
            { CPU_CAP_PG_PAT, 0x1, 0, RES_EDX, 16, 1, 0 },
            { CPU_CAP_PG_PCID, 0x1, 0, RES_ECX, 17, 1, 0 },
            { CPU_CAP_PG_SMEP, 0x7, 0, RES_EBX, 7, 1, 0 },
            { CPU_CAP_PG_NX, 0x80000001, 0, RES_EDX, 20, 1, 0 },
            { CPU_CAP_PG_1GB, 80000001, 0, RES_EDX, 26, 1, 0 },
            { CPU_CAP_PG_WIDTH_PHYS, 0x80000008, 0, RES_EAX, 0, 8, 36 },
            { CPU_CAP_PG_WIDTH_LIN, 0x80000008, 0, RES_EAX, 8, 8, 32 },
        };

        for (auto &feature: features) {
            if (feature.cap == cap) {
                *feat = feature;
                return true;
            }
        }
        return false;
    }
};

} /* namespace cpu */

#ifdef AUTONOMOUS_LINKING
}
#endif /* AUTONOMOUS_LINKING */

#endif /* MD_CPU_CAPS_H_ */
