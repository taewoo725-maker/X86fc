#!/usr/bin/env python3
# X86FC.HGL 생성: Galmuri9.bdf 의 한글 완성형 11172자(U+AC00~U+D7A3) 를 글자마다 9x9 = 9줄 x 2바이트(왼쪽 점 = 비트 15) 로 이은 파일
#   사용: python3 mkhgl.py [BDF경로] [출력경로]
#   글꼴 라이선스: Galmuri (SIL OFL 1.1, Lee Minseo)
import sys
BDF = sys.argv[1] if len(sys.argv) > 1 else '/mnt/h/msx/work/src_8.5.3/msxemu/third_party/galmuri/Galmuri9.bdf'
OUT = sys.argv[2] if len(sys.argv) > 2 else 'X86FC.HGL'
lines = open(BDF, encoding='utf-8', errors='replace').read().split('\n')
glyphs = {}
cp = None; bbx = None; i = 0
while i < len(lines):
    l = lines[i]
    if l.startswith('ENCODING'):
        cp = int(l.split()[1])
    elif l.startswith('BBX'):
        bbx = [int(v) for v in l.split()[1:5]]
    elif l.startswith('BITMAP'):
        rows = []
        i += 1
        while not lines[i].startswith('ENDCHAR'):
            s = lines[i].strip()
            rows.append((int(s or '0', 16), len(s) * 4))
            i += 1
        if cp is not None and 0xAC00 <= cp <= 0xD7A3:
            glyphs[cp] = (bbx, rows)
        cp = None
    i += 1
out = bytearray()
for u in range(0xAC00, 0xD7A4):
    cell = [0] * 9
    g = glyphs.get(u)
    if g:
        (w, h, xo, yo), rows = g
        top = 9 - h - yo                      # 9x9 칸 맨 위 줄 번호 (아래 어긋남 yo 만큼 올림)
        for r, (v, nb) in enumerate(rows):
            y = top + r
            if not 0 <= y < 9: continue
            for c in range(w):
                if v & (1 << (nb - 1 - c)):
                    x = xo + c
                    if 0 <= x < 9: cell[y] |= 0x8000 >> x
    for v in cell:
        out += bytes((v >> 8, v & 255))
# 뒤에 EUC-KR(KS X 1001 한글 2350자) 표: 체크점 37 x 2 바이트 (순번 64 의 배수의 idx) + 차이 2350 바이트 (앞 글자와의 idx 차, 첫 칸 0)
#   순번 n = (첫 바이트-B0h) x 94 + (둘째 바이트-A1h),  idx(n) = 체크점[n>>6] + 차이[(n AND NOT 63)+1 .. n] 의 합
idxs = []
for b1 in range(0xB0, 0xC9):
    for b2 in range(0xA1, 0xFF):
        idxs.append(ord(bytes([b1, b2]).decode('euc-kr')) - 0xAC00)
assert len(idxs) == 2350 and all(b - a < 256 for a, b in zip(idxs, idxs[1:]))
for j in range(37):
    out += idxs[64 * j].to_bytes(2, 'little')
out += bytes([0]) + bytes(b - a for a, b in zip(idxs, idxs[1:]))
open(OUT, 'wb').write(out)
print('ok', OUT, len(out), 'glyphs', len(glyphs))
