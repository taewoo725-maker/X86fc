; 만든이: KimTaeWoo
; X86FC SCREEN 7 한글 모드 (7H) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; F4 로 화면 방식을 0 (글자) -> 7 (SCREEN 7) -> 7H (SCREEN 7 한글) 순서로 바꿈.
;   7H = 줄 높이 10점 (21줄), 목록 15줄.  글자 칸은 가로 6점 80칸 그대로이고, 한글 한 글자 = 글자 칸 2개(12점) 에
;   Galmuri9 점글꼴 9x9 를 그림 (글자 9점 + 간격 3점).  ASCII 는 기존 6x8 글꼴을 줄 안에서 1점 내려 그림.
;   X86FC.HGL = 한글 완성형 11172자(U+AC00~U+D7A3) x 18 바이트 (9줄 x 2바이트, 왼쪽 점 = 비트 15).  X86FC.COM 옆에 둠.
; 내부 표현: 7H 일 때 lf_conv 가 긴 이름의 한글 음절을 두 바이트로 바꿈:
;   idx = U - AC00 (0~11171), 첫 바이트 = 80h + (idx >> 7) (80h~D7h), 둘째 바이트 = 80h + (idx AND 7Fh).
;   그리기에서 짝 없는 80h 이상 바이트는 빈칸.  7H 가 아닌 곳(SCREEN 7 의 8점 줄)에서는 80h 이상 바이트를 '?' 로 보임.
; 글꼴 읽기: HGL 파일 핸들을 7H 동안 열어 두고 (프로그램 실행·끝낼 때·7H 를 떠날 때 닫음), 글자 하나 = _SEEK + _READ 18 바이트.
;   VRAM 캐시 (쪽 0 의 DC00h~EFFFh): 태그 256칸 x 2 바이트 (DC00h) + 글자 256칸 x 18 바이트 (DE00h~), 칸 = idx AND FFh, 태그 FFFFh = 빈칸.
; ----------------------------------------------------------------------
S7H_Y0          equ     1                       ; 7H 줄 맨 위 여백 (줄 y = 줄 번호 x 10 + 1)
HG_TAG          equ     0DC00h
HG_DAT          equ     0DE00h
HG_NUM          equ     11172

; ---- 7H 켜기·끄기를 s7_want 에 맞춤: 줄 수 변수·긴 이름 변환·커서 보이게
h_setmode:      ld      a,(s7_want)
                or      a
                ld      a,0
                jr      z,h7s_m
                inc     a
h7s_m:          ld      (s7_thm),a
                ld      a,(s7_want)             ; (0.12.16) 줄 표 = 0 글자 / 1 SCREEN 7 (26줄) / 2 7H (10점 줄) / 3 7H (9점 줄, INI LINES = tight)
                ld      b,a
                sub     2
                ld      a,0
                jr      nz,h7s_a
                inc     a
h7s_a:          ld      hl,s7_hm
                ld      c,(hl)                  ; (0.12.54) 이전 한글 여부를 기억 (h_ohm): 한글 여부가 그대로면(0 <-> 7) 긴 이름을 디스크에서 다시 읽지 않음
                ld      (hl),a
                ld      a,c
                ld      (h_ohm),a
                ld      a,b
                cp      2
                jr      nz,h7s_n
                ld      a,(s7_tight)
                add     a,2
h7s_n:          ld      hl,s7_vm
                cp      (hl)
                ret     z                       ; 이미 그 방식
                ld      (hl),a
                ld      b,a
                ld      hl,s7_ph                ; 줄 높이·여백·한글 위치·세로선 아래 이음
                ld      de,hv_ph10
                cp      3
                jr      nz,h7s_p
                ld      de,hv_ph9
h7s_p:          ex      de,hl
                ld      bc,6
                ldir
                ld      a,(s7_vm)
                ld      b,a
                ld      hl,hv_norm
                ld      de,5
                ld      a,b
h7s_t:          or      a
                jr      z,h7s_v
                add     hl,de
                dec     a
                jr      h7s_t
h7s_v:           ld      de,v_lrows
                ld      bc,5
                ldir
                ld      a,(s7_vm)               ; (0.12.31) 방식별 줄·선 띠 높이
                ld      l,a
                ld      h,0
                ld      e,l
                ld      d,h
                add     hl,hl
                add     hl,hl
                add     hl,de
                add     hl,hl
                ld      de,hv_geo-10            ; (방식 0 = 글자 화면은 표가 없음: 앞 10 바이트는 읽어도 쓰지 않음)
                add     hl,de
                ld      de,s7_geo
                ld      bc,10
                ldir
                call    h_ytab
                ld      a,(s7_on)
                or      a
                ld      a,0
                jr      z,h7s_h
                ld      a,(s7_hm)
h7s_h:           ld      (s7_h),a
                ld      ix,pstate
                call    h_panel
                ld      ix,pstate+PSZ
; IX = 패널: 긴 이름을 새 방식(7H 면 한글 쌍)으로 다시 읽고 커서가 보이게 함
; (0.12.54) 한글 여부가 그대로면(0 <-> 7) 이름은 그대로 두고 커서만 (줄 수가 바뀌었으니) 보이게
h_panel:        ld      a,(h_ohm)
                ld      hl,s7_hm
                cp      (hl)
                jr      z,hp_cur
                ld      l,(ix+PS_LNB)
                ld      h,(ix+PS_LNB+1)
                ld      (ix+PS_LNP),l
                ld      (ix+PS_LNP+1),h
                ld      c,(ix+PS_COUNT)
                ld      b,(ix+PS_COUNT+1)
                ld      l,(ix+PS_ENT)
                ld      h,(ix+PS_ENT+1)
                ld      de,EN_LFN
                add     hl,de
h7p_c:           ld      a,b
                or      c
                jr      z,h7p_d
                xor     a
                ld      (hl),a
                inc     hl
                ld      (hl),a
                dec     hl
                ld      de,ENSZ
                add     hl,de
                dec     bc
                jr      h7p_c
h7p_d:           push    ix
                call    lfn_scan
                pop     ix
hp_cur:         ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                jp      set_cur
h_ohm:          db      0                       ; 이전 한글 여부 (h_setmode 가 정함)

; 팝업·도움말·전체 화면 오버레이는 8점 줄 격자로 그림 (7H 깃발 s7_h 를 잠깐 끔)
h_off:          xor     a
                ld      (s7_h),a
                ret
; 8점 줄 그리기를 마치면 7H 로 되돌림 (A 와 다른 레지스터 보존).  호출한 쪽이 draw_all 로 다시 그림
h_on:           push    af
                ld      a,(s7_on)
                or      a
                jr      z,h7o_x
                ld      a,(s7_ovb)              ; (0.12.33) 균일 격자로 화면을 쓴 전체 화면 오버레이(ovb = 1)만 해시를 비워 다시 그림.
                ld      (s7_dirty),a            ; 팝업 오버레이(ovb = 0)는 팝업이 덮은 줄만 다시 그림 (pop_y0~pop_y1)
                xor     a
                ld      (s7_ovb),a
                ld      a,(s7_thm)
                ld      (s7_th),a
                ld      a,(s7_hm)
                ld      (s7_h),a
h7o_x:          pop     af
                ret
ov_ret:         jr      h_on                    ; (오버레이가 ret 하면 여기로: h_on 이 ret)
; 팝업 상자 한 줄 쓰기: 8점 줄 격자로 찍고 7H 로 되돌림
; (0.12.33) 얇은 선 배치 위에 그리는 팝업은 해시를 건드리지 않고(swp_f) 덮은 y 범위만 pop_y0~pop_y1 에 기억해,
;  닫은 뒤 draw_all 이 그 범위와 겹치는 줄(s7_popinv)만 다시 그림 (예전: 전체 해시 무효 → 화면 전체가 깜박임)
scr_writep:     ld      a,(s7_th)               ; 팝업 글: 균일 8점 격자로 (얇은 선 배치와 7H 격자를 잠깐 끔)
                ld      (swp_t),a
                ld      a,(s7_h)
                or      a
                jr      nz,swp_7
                ld      a,(swp_t)
                or      a
                jp      z,scr_write
                xor     a
                ld      (s7_th),a
                inc     a
                ld      (swp_f),a
                call    pw_mark
                call    scr_write
                xor     a
                ld      (swp_f),a
                ld      a,(swp_t)
                ld      (s7_th),a
                ret
swp_7:          xor     a
                ld      (s7_th),a
                ld      (s7_h),a
                inc     a
                ld      (swp_f),a
                ld      (s7_hp),a               ; (0.11) 8점 줄이지만 한글 쌍은 9x9 글꼴로
                call    pw_mark
                call    scr_write
                xor     a
                ld      (s7_hp),a
                ld      (swp_f),a
                inc     a
                ld      (s7_h),a
                ld      a,(swp_t)
                ld      (s7_th),a
                ret
; HL = 팝업 줄의 글자 칸 주소 (줄 x 80) → 그 줄이 덮는 y 범위(2 + 줄 x 8 ~ +8)를 pop_y0~pop_y1 에 넓혀 둠 (레지스터 보존)
pw_mark:        push    hl
                push    de
                push    bc
                ld      de,80
                ld      c,0
pm_l:           or      a
                sbc     hl,de
                jr      c,pm_d
                inc     c
                jr      pm_l
pm_d:           ld      a,c
                add     a,a
                add     a,a
                add     a,a
                add     a,2                     ; (S7_Y0)
                ld      hl,pop_y0
                cp      (hl)
                jr      nc,pm_a
                ld      (hl),a
pm_a:           add     a,8
                ld      hl,pop_y1
                cp      (hl)
                jr      c,pm_e
                ld      (hl),a
pm_e:           pop     bc
                pop     de
                pop     hl
                ret
; draw_all: 팝업이 덮은 y 범위와 겹치는 줄(얇은 선 배치의 줄 표 ytab 기준)만 해시를 비움
s7_popinv:      ld      a,(s7_on)
                or      a
                jr      z,pi_r
                ld      e,0
pi_l:           ld      l,e
                ld      h,0
                ld      bc,ytab
                add     hl,bc
                ld      b,(hl)                  ; B = 줄 맨 위 y
                ld      a,(v_rfkey)
                cp      e
                ld      a,b
                jr      nz,pi_n1
                add     a,9                     ; 맨 아래 줄은 다음 표가 없으니 9점으로
                jr      pi_c
pi_n1:          inc     hl
                ld      a,(hl)                  ; A = 줄 끝 y (다음 줄 맨 위)
pi_c:           ld      c,a
                ld      a,b
                ld      hl,pop_y1
                cp      (hl)
                jr      nc,pi_nx                ; 줄 위 >= 팝업 끝: 안 겹침
                ld      a,c
                ld      hl,pop_y0
                cp      (hl)
                jr      c,pi_nx
                jr      z,pi_nx                 ; 줄 끝 <= 팝업 시작: 안 겹침
                ld      l,e
                ld      h,0
                add     hl,hl
                add     hl,hl
                ld      bc,s7_hash
                add     hl,bc
                ld      a,0FFh
                ld      (hl),a
                inc     hl
                ld      (hl),a
                inc     hl
                ld      (hl),a
                inc     hl
                ld      (hl),a
pi_nx:          inc     e
                ld      a,(v_rfkey)
                cp      e
                jr      nc,pi_l
pi_r:           xor     a
                ld      (pop_y1),a
                dec     a
                ld      (pop_y0),a
                ret
; draw_all 맨 앞: 팝업이 8점 줄로 덮었으면 7H 줄 해시를 비움
s7_undirty:     xor     a
                ld      (s7_dirty),a
                call    pi_r                    ; (0.12.33) 전체를 비우니 팝업 범위 기억도 지움
                ld      a,(s7_on)
                or      a
                ret     z
                ld      hl,s7_hash
                ld      de,s7_hash+1
                ld      (hl),0FFh
                ld      bc,111
                ldir
                ret

; ---- 긴 이름 변환 (제자리): LFNBUF 의 UCS-2 (2바이트씩, 0000 = 끝) -> 화면 글자 ASCIIZ
;      0000 아닌 Latin-1 = l1_map, 7H 이면 한글 음절 = 두 바이트 쌍, 그 밖 = '?'.  쓴 바이트가 LFN_MAX-1 에 이르면 끝냄
lf_conv:        ld      hl,LFNBUF
                ld      de,LFNBUF
                ld      b,0                     ; 쓴 바이트 수
lc_l:           ld      c,(hl)
                inc     hl
                ld      a,(hl)
                inc     hl
                ld      (lc_hi),a
                or      c
                jr      z,lc_end
                ld      a,b
                cp      LFN_MAX-1
                jr      nc,lc_end
                ld      a,(s7_hm)
                or      a
                jr      z,lc_one
                ld      a,(lc_hi)
                cp      0ACh
                jr      c,lc_one
                cp      0D7h
                jr      c,lc_pair               ; AC00~D6FF
                jr      nz,lc_one               ; D800 이상
                ld      a,c
                cp      0A4h
                jr      nc,lc_one               ; D7A4 이상은 한글 음절이 아님
lc_pair:        ld      a,(lc_hi)
                sub     0ACh
                add     a,a                     ; (hi - AC) x 2 + (lo >> 7) = idx >> 7
                bit     7,c
                jr      z,lc_p1
                inc     a
lc_p1:          add     a,80h
                ld      (de),a
                inc     de
                ld      a,c
                or      80h                     ; 80h + (lo AND 7Fh)
                ld      (de),a
                inc     de
                inc     b
                inc     b
                jr      lc_l
lc_one:         ld      a,(lc_hi)
                or      a
                ld      a,'?'                   ; Latin-1 밖의 글자
                jr      nz,lc_st
                ld      a,c
                call    l1_map
lc_st:          ld      (de),a
                inc     de
                inc     b
                jr      lc_l
lc_end:         xor     a
                ld      (de),a
                ret
lc_hi:          db      0

; ---- 글꼴 파일 X86FC.HGL 열기 (이미 열려 있으면 그대로) -> Z = 성공
hgl_open:       ld      a,(hgl_h)
                inc     a                       ; FFh = 닫힘
                jr      z,h7o_op
                xor     a
                ret
h7o_op:         ld      hl,x_ovl                ; (0.12.14) 글꼴은 X86FC.OVL 뒤에 붙어 있음 (OVL 칸 6개 뒤 = HGB)
                call    xpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (hgl_h),a
                ld      de,0
                ld      hl,OVN*OVSZ
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(hgl_h)
                ld      b,a
                ld      de,hgbuf
                ld      hl,16
                ld      c,F_READ
                call    BDOS
                ld      hl,hgbuf                ; 표식 "X86FC-HGL" 이 있어야 함 (옛 구조 OVL 이면 7H 불가)
                ld      de,txt_hgm
                ld      b,9
hgo_c:          ld      a,(de)
                cp      (hl)
                jr      nz,hgo_x
                inc     hl
                inc     de
                djnz    hgo_c
                xor     a
                ret
hgo_x:          call    hgl_close
                or      1
                ret
txt_hgm:        db      "X86FC-HGL"
hgl_close:      ld      a,(hgl_h)
                cp      0FFh
                ret     z
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                ld      a,0FFh
                ld      (hgl_h),a
                ret

; ---- VRAM 주소 지정 (HL = 0~FFFFh, 쪽 0): R#14 = 윗부분.  호출 전 di, 뒤에 ei
hgv_r:          ld      c,0
                jr      hgv_go
hgv_w:          ld      c,40h
hgv_go:         ld      a,h
                rlca
                rlca
                and     3
                out     (99h),a
                ld      a,8Eh
                out     (99h),a
                ld      a,l
                out     (99h),a
                ld      a,h
                and     3Fh
                or      c
                out     (99h),a
                ret

; ---- 글꼴 캐시 비우기 (태그 256 칸 = FFFFh).  S7 이 켜진 뒤에 부름
hg_cinit:       ld      hl,HG_TAG
                di
                call    hgv_w
                ei
                ld      bc,512
h7c_l:           ld      a,0FFh
                out     (98h),a
                dec     bc
                ld      a,b
                or      c
                jr      nz,h7c_l
                xor     a
                ld      b,14
                jp      vdp_wreg

; ---- 글자 하나 구하기: hg_idx (0~11171) -> hgbuf (18 바이트).  R#15 = 0, 인터럽트 켜진 상태에서만 부를 것
hg_get:         ld      hl,(hg_idx)             ; 2 갈래 캐시: 집합 = idx AND 7Fh, 칸 = 집합 x 2 (+0 또는 +1)
                ld      a,l
                and     7Fh
                add     a,a
                ld      (hg_slot),a
                call    hg_taga
                di
                call    hgv_r
                in      a,(98h)
                ld      e,a
                in      a,(98h)
                ld      d,a                     ; DE = 앞 칸 태그
                in      a,(98h)
                ld      c,a
                in      a,(98h)
                ld      b,a                     ; BC = 뒷 칸 태그
                ei
                ld      hl,(hg_idx)
                or      a
                sbc     hl,de
                jr      z,hg_hit
                ld      hl,(hg_idx)
                or      a
                sbc     hl,bc
                jr      nz,hg_miss0
                ld      hl,hg_slot
                inc     (hl)
                jr      hg_hit
hg_miss0:       ld      a,d                     ; 비운 칸(FFFFh)이 있으면 그 칸, 둘 다 차 있으면 번갈아
                and     e
                inc     a
                jr      z,hg_miss
                ld      a,b
                and     c
                inc     a
                jr      nz,hg_rr
                ld      hl,hg_slot
                inc     (hl)
                jr      hg_miss
hg_rr:          ld      a,(hg_rrb)
                xor     1
                ld      (hg_rrb),a
                jr      z,hg_miss
                ld      hl,hg_slot
                inc     (hl)
                jr      hg_miss
hg_hit:         call    hg_data                 ; 캐시에 있음
                di
                call    hgv_r
                ei
                ld      hl,hgbuf
                ld      b,18
                ld      c,98h
                inir
                jr      hg_done
hg_miss:        call    hg_file                 ; 파일에서 읽어 hgbuf 로
                call    hg_data
                di
                call    hgv_w
                ei
                ld      hl,hgbuf
                ld      b,18
                ld      c,98h
                otir
                call    hg_taga
                di
                call    hgv_w
                ei
                ld      a,(hg_idx)
                out     (98h),a
                ld      a,(hg_idx+1)
                out     (98h),a
hg_done:        xor     a
                ld      b,14
                jp      vdp_wreg                ; R#14 = 0 으로 되돌림
hg_taga:        ld      a,(hg_slot)             ; HL = 태그 주소 = DC00h + 칸 x 2
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      de,HG_TAG
                add     hl,de
                ret
hg_data:        ld      a,(hg_slot)             ; HL = 글자 주소 = DE00h + 칸 x 18
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,de
                add     hl,de
                ld      de,HG_DAT
                add     hl,de
                ret
; 파일의 idx x 18 위치에서 18 바이트 읽기 (실패하면 빈 글자)
hg_file:        ld      hl,hgbuf
                ld      de,hgbuf+1
                ld      (hl),0
                ld      bc,17
                ldir
                ld      a,(hgl_h)
                cp      0FFh
                ret     z
                ld      hl,(hg_idx)
                ld      de,0
                ld      b,4
h7f_sh:          add     hl,hl                   ; DE:HL = idx x 16
                rl      e
                rl      d
                djnz    h7f_sh
                ld      bc,(hg_idx)
                add     hl,bc
                jr      nc,h7f_a
                inc     de
h7f_a:           add     hl,bc                   ; + idx = x 18
                jr      nc,h7f_c
                inc     de
h7f_c:          ld      bc,HGB                  ; (0.12.14) OVL 안 글꼴 시작
                add     hl,bc
                jr      nc,h7f_b
                inc     de
h7f_b:           push    ix
                push    iy
                ld      a,(hgl_h)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(hgl_h)
                ld      b,a
                ld      de,hgbuf
                ld      hl,18
                ld      c,F_READ
                call    BDOS
                pop     iy
                pop     ix
                ret

; ---- 색 쌍 s7_pair 의 4 바이트 표 hp_pb (비트 둘 -> VRAM 한 바이트): 바탕 바탕, 바탕 글자, 글자 바탕, 글자 글자
hg_pb:          ld      a,(s7_pair)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_pairs
                add     hl,de
                ld      d,(hl)                  ; 글자색
                inc     hl
                ld      e,(hl)                  ; 바탕색
                ld      a,e
                rlca
                rlca
                rlca
                rlca
                ld      c,a
                or      e
                ld      (hp_pb),a
                ld      a,c
                or      d
                ld      (hp_pb+1),a
                ld      a,d
                rlca
                rlca
                rlca
                rlca
                ld      c,a
                or      e
                ld      (hp_pb+2),a
                ld      a,c
                or      d
                ld      (hp_pb+3),a
                ret

; ---- 한글 글자 hgbuf 를 (s7_x, s7_yb+1) 에 9줄 직접 씀 (12점 폭 = 6 바이트/줄).  R#15 = 0 상태에서 부름
hg_draw:        push    ix
                ld      a,9
                ld      (hd_cnt),a
                ld      a,(s7_cv+2)             ; 칸 맨 위 y (7H 와 8점 줄 모두 여기에 있음)
                ld      d,a
                ld      a,(s7_h)
                or      a
                jr      nz,hd_h7
                ld      hl,hgbuf+14             ; 팝업(8점 줄): 아래 두 줄(7·8째)을 겹쳐 한 줄로, 모두 8줄
                ld      a,(hgbuf+16)
                or      (hl)
                ld      (hl),a
                inc     hl
                ld      a,(hgbuf+17)
                or      (hl)
                ld      (hl),a
                ld      a,8
                ld      (hd_cnt),a
                jr      hd_g
hd_h7:          ld      a,(s7_gx)               ; 7H: 글자는 칸 맨 위에서 1점 아래 (9점 줄이면 0점)
                add     a,d
                ld      d,a
hd_g:           ld      hl,(s7_x)
                srl     h
                rr      l                       ; L = x / 2
                ld      h,d                     ; H = y
                ld      ix,hgbuf
hd_row:         di
                call    hgv_w
                ei
                ld      b,HP_PBH
                ld      d,(ix+0)
                ld      e,(ix+1)
                ld      a,d
                rlca
                rlca
                and     3
                add     a,HP_PBL
                ld      c,a
                ld      a,(bc)
                out     (98h),a
                ld      a,d
                rrca
                rrca
                rrca
                rrca
                and     3
                add     a,HP_PBL
                ld      c,a
                ld      a,(bc)
                out     (98h),a
                ld      a,d
                rrca
                rrca
                and     3
                add     a,HP_PBL
                ld      c,a
                ld      a,(bc)
                out     (98h),a
                ld      a,d
                and     3
                add     a,HP_PBL
                ld      c,a
                ld      a,(bc)
                out     (98h),a
                ld      a,e
                rlca
                rlca
                and     2                       ; 아홉째 점만 (x8), 열째 점은 바탕
                add     a,HP_PBL
                ld      c,a
                ld      a,(bc)
                out     (98h),a
                ld      c,HP_PBL                ; 열한째·열두째 점 = 바탕 바탕
                ld      a,(bc)
                out     (98h),a
                inc     ix
                inc     ix
                inc     h
                ld      a,(hd_cnt)
                dec     a
                ld      (hd_cnt),a
                jr      nz,hd_row
                pop     ix
                xor     a
                ld      b,14
                jp      vdp_wreg

; ---- 자료
; 방식별 s7_geo (줄 높이, 정보 줄 높이, 맨 위 선, 머리 아래 선, 목록 아래 선, 기능키 위 선: (선 위치, 띠 높이)).
;  보이는 간격 = 위 글자 셀 아래 빈 점 + 띠 위 여백 / 띠 아래 여백 + 아래 글자 셀 위 빈 점 이 모두 1점이 되게.
hv_geo:
                db      8,8, 0,2, 0,2, 0,2, 1,3       ; 1 SCREEN 7: 8점 글꼴은 아래 1점이 비고 위는 잉크 (정보 줄 화살표 아이콘은 8점 꽉 참 → 기능키 위 선은 위·아래 여백 1점)
                db      9,10, 0,1, 0,1, 0,1, 0,1      ; 2 7H: ASCII·아이콘은 위 1점·아래 1~2점이 비어 선 띠는 선 1점만 (정보 줄 한글은 아래가 선에 붙음)
                db      9,9, 0,1, 0,1, 0,1, 1,2       ; 3 7H tight: 정보 줄 화살표 아이콘이 9점 칸 아래까지 차서 기능키 위 선만 위 1점 띄움 (한글은 선에 붙음)
hv_norm:        db      LISTROWS,LISTROWS+LIST_Y,ROW_STAT,ROW_BOT,ROW_FKEY
hv_s7:          db      22,24,25,26,27          ; SCREEN 7 26줄 (8점 줄, 위 여백 2점): 목록 20줄
hv_hang:        db      20,22,23,24,25          ; (0.12.31) 7H: 목록 줄 높이 9점 + 선 띠를 줄여 목록 20줄 (맨 아래 줄은 화면 밖 1점까지)
hv_hang9:       db      20,22,23,24,25          ; 7H 9점 줄(tight): 목록 20줄
hv_ph10:        db      10,1,1,9,0,20                ; 줄 높이, 위 여백, 한글 아래로 이동, 세로선 아래 이음
hv_ph9:         db      9,2,0,8,0,22
s7_ph:          db      10
s7_yh:          db      1
s7_gx:          db      1
s7_bx:          db      9
s7_ty:          db      0                       ; (0.12.21) 얇은 선 배치의 첫 줄 y
v_urf:          db      20                      ; 오버레이용 균일 격자의 맨 아래 줄 (7H)
s7_tight:       db      0
s7_thm:         db      0                       ; 얇은 선 배치를 쓰는 방식 (SCREEN 7, 7H)
s7_th:          db      0                       ; 지금 그리는 화면이 얇은 선 배치 (팝업·오버레이를 그리는 동안은 0)
s7_ovb:         db      0                       ; 오버레이 실행 중
swp_t:          db      0
swp_f:          db      0                       ; 팝업 글을 쓰는 중: 해시를 쓰지 않음
pop_y0:         db      0FFh                    ; 팝업이 덮은 y 범위 (닫은 뒤 그 줄만 다시 그림)
pop_y1:         db      0
s7_geo:         db      8,8, 0,2, 0,2, 0,2, 0,2   ; (0.12.31) 얇은 선 배치: 보통 줄·정보 줄 높이, 맨 위·머리 아래·목록 아래·기능키 위 선 띠(선 위치, 높이)
ytab:           ds      32                      ; 줄 번호 -> y (얇은 선 배치)
v_lrows:        db      LISTROWS                ; 목록 줄 수
v_rrule:        db      LISTROWS+LIST_Y         ; 목록 아래 가로선 줄
v_rstat:        db      ROW_STAT                ; 정보 줄
v_rbot:         db      ROW_BOT                 ; 기능키 위 가로선
v_rfkey:        db      ROW_FKEY                ; 기능키 줄
s7_vm:          db      0                       ; 지금 적용된 줄 표 방식 (0 글자, 1 SCREEN 7, 2 7H)
s7_hm:          db      0                       ; 7H 방식 (s7_want = 2): 이름 변환·줄 수가 7H 용
s7_h:           db      0                       ; 지금 그리는 격자가 7H (10점 줄)
s7_hp:          db      0                       ; 팝업 글 그리는 중 (7H 의 8점 줄 격자, 한글 쌍을 그림)
s7_dirty:       db      0                       ; 팝업이 8점 줄로 덮음 -> draw_all 이 해시를 비움
s7_yb:          db      0                       ; 7H 그리는 줄의 맨 위 y
s7_cc:          db      0
s7_xf:          db      0
hgl_h:          db      0FFh                    ; X86FC.HGL 핸들 (FFh = 닫힘)
hg_idx:         dw      0
hg_slot:        db      0
hg_rrb:         db      0
hd_cnt:         db      0
hgbuf:          ds      18
hp_al:          ds      (4-(hp_al MOD 4)) MOD 4 ; (hp_pb 를 4 바이트 경계에: 표 번호를 OR/ADD 로 바로 씀)
hp_pb:          ds      4
HP_PBH          equ     hp_pb>>8
HP_PBL          equ     hp_pb MOD 256
; 줄 칸 코드 1~9 (세로선 따위)를 7H 줄에서 위·아래로 이어 그림: 비트 0 = 위 한 줄, 비트 1 = 아래 한 줄
hx_tab:         db      0,3,0,3,1,2,2,1,1,3

; ---- (0.12.31) 얇은 선 배치: 줄 번호 -> y 표.  줄 높이는 s7_geo (보통 줄, 정보 줄, 선 띠마다 다름) ----
h_ytab:         ld      a,(s7_ty)
                ld      hl,s7_geo+3             ; 맨 위 선 띠 아래에서 시작
                add     a,(hl)
                ld      d,a
                ld      e,0
                ld      hl,ytab
yt_l:           ld      (hl),d
                inc     hl
                push    hl
                ld      a,(s7_geo)              ; 보통 줄
                ld      c,a
                ld      a,e
                ld      hl,s7_geo+5
                cp      1
                jr      z,yt_b                  ; 머리 아래 선 띠
                ld      hl,s7_geo+7
                ld      b,a
                ld      a,(v_rrule)
                cp      b
                ld      a,b
                jr      z,yt_b                  ; 목록 아래 선 띠
                ld      hl,s7_geo+9
                ld      a,(v_rbot)
                cp      b
                ld      a,b
                jr      z,yt_b                  ; 기능키 위 선 띠
                ld      hl,s7_geo+1
                ld      a,(v_rstat)
                cp      b
                jr      nz,yt_a                 ; 정보 줄
yt_b:           ld      c,(hl)
yt_a:           ld      a,d
                add     a,c
                ld      d,a
                pop     hl
                inc     e
                ld      a,(v_rfkey)
                cp      e
                jr      nc,yt_l
                ret
