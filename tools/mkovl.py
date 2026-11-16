#!/usr/bin/env python3
# X86FC 오버레이 빌드: 본체를 조립해 레이블 표(ovsym.inc)를 만들고, 오버레이마다 조립해 X86FC.OVL 로 이어 붙인다.
# 사용: python3 mkovl.py <버전 폴더>      설정: <버전 폴더>/ovl.cfg  (줄마다: 번호 모듈파일 진입라벨,진입라벨)
import re, subprocess, sys, os
d = sys.argv[1]
os.chdir(d)
r = subprocess.run(['pasmo', '-d', 'x86fc.asm', '/tmp/x86fc_build.com'], capture_output=True, text=True, errors='replace')
if r.returncode != 0:
    print('본체 조립 실패:', (r.stdout + r.stderr)[:400]); sys.exit(1)
dump = r.stdout + r.stderr
cfg = []
for ln in open('ovl.cfg', encoding='utf-8', errors='replace'):
    ln = ln.strip()
    if not ln or ln.startswith('#'): continue
    n, mod, ents = ln.split()
    cfg.append((int(n), mod, ents.split(',')))
# 오버레이 안에서 정의되는 이름 (본체 쪽 껍데기와 겹치므로 ovsym 에서 뺌)
local = set()
for n, mod, ents in cfg:
    for l in open(mod, encoding='utf-8', errors='replace'):
        m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)(:|\s+equ\s)', l, re.I)
        if m: local.add(m.group(1))
syms = []
ovsz = None
for l in dump.split('\n'):
    m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*)\s+EQU\s+([0-9A-F]+)\s*$', l)
    if m:
        syms.append((m.group(1), int(m.group(2), 16)))
        if m.group(1) == 'OVSZ': ovsz = int(m.group(2), 16)
        continue
    m = re.match(r'^([0-9A-F]+):\s+label\s+([A-Za-z_][A-Za-z0-9_]*)\s*$', l)
    if m: syms.append((m.group(2), int(m.group(1), 16)))
with open('ovsym.inc', 'w', encoding='utf-8', newline='\n') as f:
    f.write('; 자동 생성 (mkovl.py): 본체의 레이블·상수 표\n')
    for n, v in sorted(set(syms)):
        if n in local: continue
        f.write('%s equ 0%04Xh\n' % (n, v))
if ovsz is None:
    print('OVSZ 를 찾지 못함'); sys.exit(1)
out = bytearray()
for n, mod, ents in sorted(cfg):
    asm = 'ov%d.asm' % n
    with open(asm, 'w', encoding='utf-8', newline='\n') as f:
        f.write('        include "ovsym.inc"\n        org     OVBASE\n')
        for en in ents:
            f.write('        jp      %s\n' % en)
        f.write('        include "%s"\n' % mod)
    r = subprocess.run(['pasmo', asm, 'ov%d.bin' % n], capture_output=True, text=True, errors='replace')
    if r.returncode != 0:
        print('오버레이 %d 조립 실패:' % n, (r.stdout + r.stderr)[:600]); sys.exit(1)
    b = open('ov%d.bin' % n, 'rb').read()
    print('오버레이 %d (%s): %d 바이트 / 칸 %d' % (n, mod, len(b), ovsz))
    if len(b) > ovsz:
        print('  칸을 넘음 — x86fc.asm 의 OVSZ 를 늘리세요'); sys.exit(1)
    out += b + bytes(ovsz - len(b))
# (0.12.14) 글꼴(hgl.bin = mkhgl.py 가 만든 X86FC.HGL)을 OVL 뒤에 붙임: 16바이트 표식 "X86FC-HGL1" + 글꼴 11172자 x 18B + EUC-KR 표.
#   NOHGL=1 이면 붙이지 않음 (회귀 시험용 작은 OVL)
hg = os.path.join(os.getcwd(), 'hgl.bin')
if os.path.exists(hg) and not os.environ.get('NOHGL'):
    out += b'X86FC-HGL1'.ljust(16, bytes(1)) + open(hg, 'rb').read()
# (0.12.34) 글꼴 없는 시험용 OVL(NOHGL)은 릴리스 폴더에 쓰지 않음 (/tmp 에만): 릴리스 OVL 은 항상 글꼴 포함
if not os.environ.get('NOHGL'):
    os.makedirs('실행파일', exist_ok=True)
    open('실행파일/X86FC.OVL', 'wb').write(out)
open('/tmp/X86FC.OVL', 'wb').write(out)
for n, mod, ents in cfg:
    os.remove('ov%d.asm' % n); os.remove('ov%d.bin' % n)
print('X86FC.OVL', len(out), '바이트')
