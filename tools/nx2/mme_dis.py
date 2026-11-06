"""Disassemble Turing/Ampere native MME code dumped by Lichii (EDEN_NX2_MME_TRACE).

Field layout and semantics follow src/video_core/macro_turing.h (from Mesa mme_tu104).
Usage: python mme_dis.py nx2-mme-code.bin nx2-mme-positions.bin <macro method hex> [max instrs]
"""
import struct
import sys

OPS = {0: "add", 1: "addc", 2: "sub", 3: "subb", 4: "mul", 5: "mulh", 6: "mulu", 7: "ext",
       8: "clz", 9: "sll", 10: "srl", 11: "sra", 12: "and", 13: "nand", 14: "or", 15: "xor",
       16: "merge", 17: "slt", 18: "sltu", 19: "sle", 20: "sleu", 21: "seq", 22: "state",
       23: "loop", 24: "jal", 25: "blt", 26: "bltu", 27: "ble", 28: "bleu", 29: "beq",
       30: "dread", 31: "dwrite"}
PRED = ["UUUU", "TTTT", "FFFF", "TTUU", "FFUU", "TFUU", "TUUU", "FUUU",
        "UUTT", "UUTF", "UUTU", "UUFT", "UUFF", "UUFU", "UUUT", "UUUF"]
OUT = {0: None, 1: "alu0", 2: "alu1", 3: "load0", 4: "load1", 5: "imm0", 6: "imm1",
       8: "imm0>>12", 9: "imm1>>12", 10: "imm32"}


def field(bits, low, count):
    value = bits[low // 32]
    if low // 32 < 2:
        value |= bits[low // 32 + 1] << 32
    return (value >> (low % 32)) & ((1 << count) - 1)


def reg(r, lane):
    if r < 24:
        return f"r{r}"
    return {24: "zero", 25: f"imm{lane}", 26: f"imm{1 - lane}", 27: "imm32",
            28: "load0", 29: "load1"}.get(r, f"?{r}")


def decode(words, pc):
    bits = [words[pc * 3 + 2], words[pc * 3 + 1], words[pc * 3]]
    imm = [field(bits, 30, 16), field(bits, 66, 16)]
    op = [field(bits, 10, 5), field(bits, 46, 5)]
    dst = [field(bits, 15, 5), field(bits, 51, 5)]
    sa = [field(bits, 20, 5), field(bits, 56, 5)]
    sb = [field(bits, 25, 5), field(bits, 61, 5)]
    om = [field(bits, 82, 3), field(bits, 89, 3)]
    oe = [field(bits, 85, 4), field(bits, 92, 4)]
    pred_reg, pred_mode, end = field(bits, 5, 5), field(bits, 1, 4), field(bits, 0, 1)
    parts = []
    if pred_mode:
        parts.append(f"@{reg(pred_reg, 0)}:{PRED[pred_mode]}")
    for lane in range(2):
        o = op[lane]
        name = OPS[o]
        if o == 0 and dst[lane] == 24 and sa[lane] == 24 and sb[lane] == 24:
            continue  # nop lane
        d = reg(dst[lane], lane)
        a, b = reg(sa[lane], lane), reg(sb[lane], lane)
        if o in (25, 26, 27, 28, 29):
            off = ((imm[lane] << 19) & 0xFFFFFFFF)
            off = (off if off < 2**31 else off - 2**32) >> 19
            take = "T" if imm[lane] & 0x8000 else "F"
            parts.append(f"{name}{take} {a},{b} -> pc{pc + off:+d}={pc + off:#x}")
        elif o == 24:
            parts.append(f"jal -> {pc + (imm[lane] & 0x7fff):#x}")
        elif o == 23:
            parts.append(f"loop {a} x body {imm[lane]}")
        elif o == 22:
            parts.append(f"{d} = state[{a}+{b}]")
        elif o == 30:
            parts.append(f"{d} = dram[{a}]")
        elif o == 31:
            parts.append(f"dram[{a}] = {b}")
        elif o == 16:
            i = imm[lane]
            parts.append(f"{d} = merge({a}, {b}, dst={(i >> 10) & 63}, n={(i >> 5) & 31}, src={i & 31})")
        else:
            parts.append(f"{d} = {name} {a},{b}" + (f" (imm {imm[lane]:#x})" if 25 in (sa[lane], sb[lane]) else ""))
    def outval(sel):
        return {5: imm[0], 6: imm[1], 8: imm[0] >> 12, 9: imm[1] >> 12,
                10: (imm[0] << 16) | imm[1]}.get(sel)

    for lane in range(2):
        if om[lane]:
            v = outval(om[lane])
            if v is not None:
                parts.append(f"mthd {v & 0xfff:#x} (+{(v >> 12) & 15})")
            else:
                parts.append(f"mthd<-{OUT.get(om[lane])}")
        if oe[lane]:
            v = outval(oe[lane])
            parts.append(f"emit {v:#x}" if v is not None else f"emit<-{OUT.get(oe[lane])}")
    return parts, end


def main():
    code = struct.unpack(f"<{len(open(sys.argv[1], 'rb').read()) // 4}I", open(sys.argv[1], "rb").read())
    pos_raw = open(sys.argv[2], "rb").read()
    positions = struct.unpack(f"<{len(pos_raw) // 4}I", pos_raw)
    method = int(sys.argv[3], 16)
    limit = int(sys.argv[4]) if len(sys.argv) > 4 else 200
    start = positions[((method - 0xE00) >> 1) % len(positions)]
    print(f"macro {method:#x} -> entry {((method - 0xE00) >> 1)} start pc {start:#x}")
    ends = 0
    for pc in range(start, min(start + limit, len(code) // 3)):
        parts, end = decode(code, pc)
        print(f"{pc:04x}: " + " ; ".join(parts) + ("   [END+1]" if end else ""))
        if end:
            ends += 1
            if ends >= 2:
                break


if __name__ == "__main__":
    main()
