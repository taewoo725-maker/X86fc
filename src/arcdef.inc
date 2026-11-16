; 만든이: KimTaeWoo
; X86FC 압축 파일 목록 보기 (오버레이 6) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; Enter 로 연결 프로그램이 없는 파일을 열 때 먼저 불림 (do_arc, RUNPATH = 파일 경로).  서명으로 형식을 가려냄:
;   LZH/LHA/PMA = 3~7번째 글자가 "-lh?-" "-lz?-" "-pm?-" (머리 수준 0·1·2),  ZIP = "PK" 3 4 (중앙 디렉터리),  ARJ = 60h EAh.
; 압축 파일이면 전체 화면 목록을 보임 (돌아올 때 Z), 아니면 아무것도 안 하고 NZ 로 돌아감 (본체가 예전 "연결할까요?" 로 이어 감).
;   ↑↓ = 한 줄, ←→ = 한 쪽, HOME = 처음, B = 끝, ESC·F10·Q = 닫기.  (풀기는 다음 판)
; 목록은 머리의 파일 위치(4 바이트)만 CPBUF 에 모아 두고, 보이는 줄만 그때그때 머리를 읽어 해석한다 (램이 모자라서).
; ----------------------------------------------------------------------
AR_OFF          equ     CPBUF                   ; 머리 위치 표 (4 바이트 x AR_MAX, S7TAB 앞까지)
AR_MAX          equ     250
AR_REC          equ     LVSTK                   ; 해석한 한 항목
R_NAME          equ     0                       ; 이름 (64, ASCIIZ)
R_OSZ           equ     64                      ; 원래 크기 (4)
R_CSZ           equ     68                      ; 압축 크기 (4)
R_DATE          equ     72                      ; DOS 날짜 (2)
R_METH          equ     74                      ; 방식 글 (6, ASCIIZ)
R_DOFF          equ     80                      ; 압축 자료 시작 위치 (4)
R_CRC           equ     84                      ; 머리의 CRC (LZH 16비트, ZIP 32비트) (4)
R_TIME          equ     88                      ; DOS 시각 (2)
R_MCH           equ     90                      ; 방식 글자 (LZH '0' '4' '5' 'd'.., ZIP 'S' 저장 'Z' deflate '?' 못 풂 'd' 폴더)
AR_ROWS         equ     20                      ; 목록 줄 수 (화면 줄 2~21)
; (0.12.39) 변수는 오버레이 안이 아니라 페이지 3 의 LVSTK 뒤(AV)에 둠: 오버레이 창 끝이 8000h 를 넘으면 풀기 동안 매퍼 쪽에 가려지기 때문
AV              equ     AR_REC+96
AVN             equ     115
ar_h            equ     AV+0
ar_fmt          equ     AV+1
ar_n            equ     AV+2
ar_zn           equ     AV+4
ar_top          equ     AV+6
ar_cur          equ     AV+8
ar_old          equ     AV+10
ar_ix           equ     AV+12
ar_sr           equ     AV+14
ar_pos          equ     AV+15
ar_cs           equ     AV+19
ar_q            equ     AV+23
ar_y            equ     AV+27
ar_seg          equ     AV+29
ar_p2           equ     AV+30
ar_h2           equ     AV+31
ar_ho           equ     AV+32
ar_res          equ     AV+33
ar_ovr          equ     AV+34
ar_t            equ     AV+35
ar_c            equ     AV+37
ar_pix          equ     AV+39
acq_m           equ     AV+41
acq_n           equ     AV+42
mp_all          equ     AV+43
mp_fre          equ     AV+45
mp_put          equ     AV+47
mp_get          equ     AV+49
ar_np           equ     AV+51
ar_et           equ     AV+53
ar_n1l          equ     AV+55
ar_n1p          equ     AV+57
ar_nx           equ     AV+59
ar_fn           equ     AV+61
ar_ptx          equ     AV+75
DEC_OFS         equ     7*OVSZ                  ; X86FC.OVL 안 오버레이 7 의 위치
DEC_RUN         equ     8000h
AR_MARK         equ     CPBUF+1000              ; 표시 비트 (250 비트 = 32 바이트)
