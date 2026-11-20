; 만든이: KimTaeWoo
; X86FC SCREEN 7 모드 (선택 모드) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; SCREEN 7 (512x212, 16색) 로 글자를 비트맵으로 찍는 화면.  글자 하나 = 6x8 점, 80칸 x 24줄,
; 그림은 (16,10) 점에서 시작.  색 조합(쌍)마다 글꼴판을 VRAM 두 번째 페이지에 미리 만들어 두고,
; 글자는 VDP 명령 HMMM 으로 복사, 빈칸 덩어리는 HMMV 로 채움 (CPU 는 명령 인자만 보냄).
; 바뀌지 않은 줄(반쪽)은 해시로 알아내서 건너뜀.
; VDP 명령 속도 (V9938, 스프라이트 끄고 212줄, 60Hz): HMMM 약 3600 바이트/프레임, HMMV 약 5900 바이트/프레임.
; 쓰기 전 R#15=2 로 두고 CE(S#2 비트 0) 를 기다림, 그 동안 인터럽트 금지 (BIOS 인터럽트가 S#0 을 읽기 때문).
; ----------------------------------------------------------------------
S7_X0           equ     16
S7_Y0           equ     2                       ; (0.12.16) 위 여백 2점 -> 26줄 x 8점 = 208점
NPAIR           equ     12
S7FNT           equ     CPBUF                   ; 글꼴 1024 바이트 (잠깐 씀)
S7TAB           equ     ((CPBUF+1024+255)/256)*256 ; 표 3쪽 (256 칸 맞춤)
S7TABH          equ     S7TAB>>8
S7LBE           equ     (s7_lb+48) MOD 256      ; 한 줄 조립 끝 (낮은 바이트)
S7_ICON         equ     10h                    ; 아이콘 글자 번호 10h~18h

; ---- 켜기 (S0 80칸 글자 화면이 준비된 뒤): 글꼴을 읽고 SCREEN 7 로 바꾸고 글꼴판을 만듦
s7_enter:       ld      a,(vdp_dat)
                cp      98h
                jr      z,s7e_ok
                ld      a,(s7_want)
                cp      2
                ret     nz
                ld      a,1                     ; 표준 포트가 아니면 7H 도 못 씀
                ld      (s7_want),a
                jp      h_setmode
s7e_ok:         di
                xor     a
                out     (99h),a
                ld      a,10h
                out     (99h),a                 ; VRAM 읽기 주소 1000h (S0 80칸 글꼴)
                ei
                ld      hl,S7FNT
                ld      de,1024
s7e_r:          in      a,(98h)
                ld      (hl),a
                inc     hl
                dec     de
                ld      a,d
                or      e
                jr      nz,s7e_r
                ld      hl,s7_icons             ; 아이콘 9개를 10h~18h 자리에 덮어씀
                ld      de,S7FNT+S7_ICON*8
                ld      bc,12*8
                ldir
                call    dos_fetch               ; 맡겨 둔 DOS 글을 SECBUF 로 (CHGMOD 가 VRAM 을 지우므로)
                ld      a,7
                ld      ix,CHGMOD
                call    bios
                call    s7_pal
                ld      a,1
                ld      b,7
                call    vdp_wreg                ; 테두리 색
                ld      a,(0FFE7h)              ; R#8 스프라이트 끔 (명령이 빨라짐)
                or      02h
                ld      (0FFE7h),a
                ld      b,8
                call    vdp_wreg
                call    s7_build
                ld      a,1
                ld      (s7_on),a
                call    dos_stash7
                xor     a
                ld      (s7_pair),a
                ld      a,(s7_want)
                cp      2
                jr      nz,s7e_n
                call    hgl_open                ; 7H: 글꼴 파일을 열고 글꼴 캐시를 비움
                jr      z,s7e_h
                ld      a,1
                ld      (s7_want),a
                call    h_setmode
                jr      s7e_n
s7e_h:          call    hg_cinit
s7e_n:          ld      a,(s7_hm)
                ld      (s7_h),a
                jp      s7_clear

; ---- 끄기 표시 (VDP 는 다음 CHGMOD 가 되돌림): 팔레트 처음대로
s7_leave:       ld      a,(s7_on)
                or      a
                ret     z
                xor     a
                ld      (s7_on),a
                ld      (s7_h),a
                ld      a,(0FFE7h)
                and     0FDh
                ld      (0FFE7h),a
                ld      hl,s7_defpal            ; 팔레트를 MSX2 기본값으로 (BIOS INIPLT 는 에뮬레이터에서 안 먹어 직접 씀)
                jr      s7_pw

; ---- 글자 화면으로 돌아감 (F9)
s7_back:        ld      a,(s7_on)
                or      a
                ret     z
                call    dos_fetch
                call    s7_leave
                call    scr_text
                jp      dos_stash0

; ---- F9: 화면 방식 바꿈
ml_mode:        ld      a,(s7_want)
                inc     a
                cp      3
                jr      c,mm_n
                xor     a
mm_n:           cp      2
                jr      nz,mm_set
                call    hgl_open                ; 7H 는 X86FC.HGL 이 있을 때만 (없으면 건너뜀: 0 -> 7 -> 0)
                ld      a,2
                jr      z,mm_set
                xor     a
mm_set:         ld      (s7_want),a
                or      a
                jr      z,mm_off
                ld      a,(s7_on)
                or      a
                jr      nz,mm_on
                call    s7_enter                ; 0 -> 7
                call    h_setmode               ; (0.12.30) 줄 표·얇은 선 배치(맨 위 선 포함)를 이 방식에 맞춤: 빠뜨리면 F4 로 0 -> 7 일 때 틀이 없는 격자가 됨
                jr      mm_dr
mm_on:          call    h_setmode               ; 7 -> 7H: 줄 높이·이름 변환
                call    hg_cinit
                ld      a,(s7_hm)
                ld      (s7_h),a
                call    s7_clear
                jr      mm_dr
mm_off:         call    hgl_close               ; 7H -> 0
                call    s7_back
                call    h_setmode
mm_dr:          xor     a
                ld      (si_c2),a
                ld      a,(s7_on)
                or      a
                ld      a,'0'
                jr      z,mm_dq
                ld      a,'7'
                ld      hl,s7_want
                bit     1,(hl)
                jr      z,mm_dq
                ld      hl,si_c2
                ld      (hl),'H'
mm_dq:          ld      (si_ch),a
                call    draw_all                ; (0.12.54) 화면을 먼저 그리고 설정 파일(INI)을 씀 (디스크 쓰기를 기다리는 동안 빈 화면이 보이지 않게)
                call    s7_setini
                jp      ml_next

s7_pal:         ld      hl,s7_palette
s7_pw:          di
                xor     a
                out     (99h),a
                ld      a,90h
                out     (99h),a
                ld      bc,0209Ah
                otir
                ei
                ret

; ---- 색 표 만들기: A = 글자색, B = 바탕색 (S7TAB 쪽 3장: 글꼴 한 줄 → 바이트 3개)
s7_mktab:       ld      d,a
                ld      e,b
                ld      a,b
                rlca
                rlca
                rlca
                rlca
                ld      c,a
                or      b
                ld      (s7_pb),a               ; 바탕 바탕
                ld      a,c
                or      d
                ld      (s7_pb+1),a             ; 바탕 글자
                ld      a,d
                rlca
                rlca
                rlca
                rlca
                ld      c,a
                or      e
                ld      (s7_pb+2),a             ; 글자 바탕
                ld      a,c
                or      d
                ld      (s7_pb+3),a             ; 글자 글자
                ld      c,0
s7m_l:          ld      a,c
                add     a,a
                add     a,a
                ld      e,a
                ld      d,S7TABH
                ld      a,c
                rrca
                rrca
                rrca
                rrca
                and     3
                call    s7_pbget
                ld      (de),a
                inc     d
                ld      a,c
                rrca
                rrca
                and     3
                call    s7_pbget
                ld      (de),a
                inc     d
                ld      a,c
                and     3
                call    s7_pbget
                ld      (de),a
                inc     c
                ld      a,c
                cp      64
                jr      nz,s7m_l
                ret
s7_pbget:       ld      hl,s7_pb
                add     a,l
                ld      l,a
                jr      nc,s7p_1
                inc     h
s7p_1:          ld      a,(hl)
                ret

; ---- 글꼴판 12장을 VRAM 두 번째 페이지(Y 256~447)에 만듦
s7_build:       xor     a
                ld      (s7b_p),a
s7b_pl:         ld      a,(s7b_p)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_pairs
                add     hl,de
                ld      a,(hl)
                inc     hl
                ld      b,(hl)
                call    s7_mktab
                ld      a,(s7b_p)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_sxt
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                srl     b
                rr      c
                ld      a,c
                ld      (s7b_xb),a              ; 글꼴판 왼쪽 (바이트)
                ld      hl,s7_syt
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ld      (s7b_y),de              ; 글꼴판 맨 위 줄 (Y)
                xor     a
                ld      (s7b_n),a
s7b_nl:         ld      a,(s7b_n)               ; 0~63 = 글자 줄(0~7) x 점 줄(0~7)
                and     0F8h
                ld      l,a
                ld      h,0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; (n>>3)*128
                ld      a,(s7b_n)
                and     7
                ld      e,a
                ld      d,0
                add     hl,de
                ld      de,S7FNT
                add     hl,de
                ld      de,s7_lb
                ld      a,(s7b_n)
                and     0F8h
                cp      8
                jr      z,s7b_ic                ; 글자 줄 1 = 아이콘 (8점 폭)
s7b_c:          ld      a,(hl)
                and     0FCh
                ld      c,a
                ld      b,S7TABH
                ld      a,(bc)
                ld      (de),a
                inc     de
                inc     b
                ld      a,(bc)
                ld      (de),a
                inc     de
                inc     b
                ld      a,(bc)
                ld      (de),a
                inc     de
                ld      a,l
                add     a,8
                ld      l,a
                jr      nc,s7b_c2
                inc     h
s7b_c2:         ld      a,e
                cp      S7LBE
                jr      nz,s7b_c
                jr      s7b_wr
s7b_ic:         ld      a,(hl)                  ; 아이콘 한 점 줄: 윗 6 비트 → 3 바이트, 아랫 2 비트 → 1 바이트
                and     0FCh
                ld      c,a
                ld      b,S7TABH
                ld      a,(bc)
                ld      (de),a
                inc     de
                inc     b
                ld      a,(bc)
                ld      (de),a
                inc     de
                inc     b
                ld      a,(bc)
                ld      (de),a
                inc     de
                ld      a,(hl)
                and     3
                rrca
                rrca
                ld      c,a
                ld      b,S7TABH
                ld      a,(bc)
                ld      (de),a
                inc     de
                ld      a,l
                add     a,8
                ld      l,a
                jr      nc,s7b_i2
                inc     h
s7b_i2:         ld      a,e
                cp      S7LBE
                jr      nz,s7b_ic
s7b_wr:         ld      hl,(s7b_y)              ; VRAM 에 쓰기 (한 줄 48 바이트)
                ld      a,l
                rlca
                rlca
                and     3
                ld      b,a
                ld      a,h
                add     a,a
                add     a,a
                or      b
                di
                out     (99h),a
                ld      a,8Eh
                out     (99h),a
                ld      a,(s7b_xb)
                out     (99h),a
                ld      a,l
                and     3Fh
                or      40h
                out     (99h),a
                ei
                ld      hl,s7_lb
                ld      bc,3098h
                otir
                ld      hl,(s7b_y)
                inc     hl
                ld      (s7b_y),hl
                ld      a,(s7b_n)
                inc     a
                ld      (s7b_n),a
                cp      64
                jp      nz,s7b_nl
                ld      a,(s7b_p)
                inc     a
                ld      (s7b_p),a
                ld      hl,s7_np
                cp      (hl)
                jp      nz,s7b_pl
                xor     a
                ld      b,14
                jp      vdp_wreg                ; R#14 = 0 으로 되돌림

; ---- 화면 전체를 바탕색으로
s7_clear:       di
                ld      a,2
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                call    s7_wait
                ld      a,36
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cvall
                ld      bc,0B9Bh
                otir
                call    s7_wait
                xor     a
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                ei
                ld      hl,s7_hash
                ld      de,s7_hash+1
                ld      (hl),0FFh
                ld      bc,111
                ldir
                ret
s7_wait:        in      a,(99h)
                rrca
                jr      c,s7_wait
                ret

; ---- 글 찍기: HL = 글자 칸 번호 (줄 x 80 + 칸), DE = 글, B = 개수  (scr_write 의 S7 판)
s7_write:       ld      (s7_tp),de
                ld      a,b
                ld      (s7_n),a
                ld      de,80
                ld      c,0
s7w_d:          or      a
                sbc     hl,de
                jr      c,s7w_e
                inc     c
                jr      s7w_d
s7w_e:          add     hl,de
                ld      a,c
                ld      (s7_row),a
                ld      a,l
                ld      (s7_col),a
                ld      a,(s7_th)
                or      a
                jr      z,s7w_n
                ld      a,(s7_row)
                cp      1
                ret     z                       ; (0.12.21) 선 줄은 s7_bands 가 띠로 그림
                ld      hl,v_rrule
                cp      (hl)
                ret     z
                ld      hl,v_rbot
                cp      (hl)
                ret     z
s7w_n:          ld      a,(swp_f)               ; (0.12.33) 팝업 글은 해시 없이 바로 그림
                or      a
                jr      nz,s7_draw
                call    s7_hslot
                or      a
                jr      z,s7_draw
                push    hl
                call    s7_hh
                ex      de,hl
                pop     hl
                ld      a,(hl)
                cp      e
                jr      nz,s7w_s
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      d
                ret     z                       ; 그대로이니 건너뜀
s7w_s:          ld      (hl),e
                inc     hl
                ld      (hl),d
                ld      hl,(s7_hx)
                ld      a,h
                or      l
                jr      z,s7_draw
                ld      (hl),0FFh
                inc     hl
                ld      (hl),0FFh

; ---- 찍기 본체: s7_row, s7_col, s7_tp, s7_n, s7_pair 로
s7_draw:        xor     a
                ld      (s7_icn),a
                ld      a,(s7_col)
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de                   ; 칸 x 6
                ld      de,S7_X0
                add     hl,de
                ld      de,(s7_xo)              ; (0.12.22) x 보정 (오른쪽 아이콘)
                add     hl,de
                ld      (s7_x),hl
                ld      a,(s7_th)               ; (0.12.21) 얇은 선 배치면 y 는 표에서
                or      a
                jr      z,s7d_u
                ld      a,(s7_row)
                ld      e,a
                ld      d,0
                ld      hl,ytab
                add     hl,de
                ld      c,(hl)
                ld      a,(s7_h)
                or      a
                jr      nz,s7t_7
                ld      a,c
                ld      (s7_cb+6),a
                ld      (s7_cv+2),a
                ld      a,8
                ld      (s7_cv+6),a
                jr      s7d_yc
s7t_7:          ld      a,c
                ld      (s7_yb),a
                ld      (s7_cv+2),a
                inc     a
                ld      (s7_cb+6),a
                ld      a,(s7_row)              ; (0.12.31) 얇은 선 배치의 칸 높이: 한글이 나오는 정보 줄만 s7_geo+1, 나머지는 s7_geo (7H 9점)
                ld      hl,v_rstat
                cp      (hl)
                ld      a,(s7_geo)
                jr      nz,s7t_8
                ld      a,(s7_geo+1)
s7t_8:          ld      (s7_cv+6),a
                jr      s7d_yc
s7d_u:          ld      a,(s7_h)
                or      a
                jr      nz,s7d_y10
                ld      a,(s7_row)
                add     a,a
                add     a,a
                add     a,a
                add     a,S7_Y0
                ld      (s7_cb+6),a
                ld      (s7_cv+2),a
                ld      a,8
                ld      (s7_cv+6),a
                jr      s7d_yc
s7d_y10:        ld      a,(s7_row)              ; 7H: 줄 맨 위 = 줄 x 10 + 1, 글자는 1점 아래
                ld      c,a                     ; (0.12.16) 줄 y = 줄 x 피치(10 또는 9) + 위 여백
                ld      b,a
                ld      a,(s7_ph)
                cp      10
                jr      nz,s7d_t9
                ld      a,c
                add     a,a
                ld      b,a
s7d_t9:         ld      a,c
                add     a,a
                add     a,a
                add     a,a
                add     a,b
                ld      hl,s7_yh
                add     a,(hl)
                ld      (s7_yb),a
                ld      (s7_cv+2),a
                inc     a
                ld      (s7_cb+6),a
                ld      a,(s7_ph)
                ld      (s7_cv+6),a
s7d_yc:
                ld      a,(s7_pair)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_sxt
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                ld      (s7_sx0),bc
                ld      hl,s7_syt
                add     hl,de
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                ld      (s7_sy0),bc
                ld      hl,s7_pairs+1
                add     hl,de
                ld      a,(hl)                  ; 바탕색
                ld      e,a
                rlca
                rlca
                rlca
                rlca
                or      e
                ld      (s7_cv+8),a             ; 채울 바이트 = 바탕색 x 17
                di
                ld      a,2
                out     (99h),a
                ld      a,8Fh
                out     (99h),a                 ; R#15 = 2 (S#2 읽기)
                ld      a,(s7_h)
                or      a
                call    nz,s7d_pre              ; 7H: 칸 전체를 먼저 바탕색으로 칠함
s7d_l:          ld      a,(s7_n)
                or      a
                jp      z,s7d_end
                ld      hl,(s7_tp)
                ld      a,(hl)
                cp      80h
                jr      c,s7d_a1
                ld      a,(s7_h)
                ld      b,a
                ld      a,(s7_hp)
                or      b
                jp      nz,s7d_hg               ; 7H 한글 쌍 (팝업 글도)
                ld      a,'?'                   ; 8점 줄: 80h 이상은 ?
s7d_a1:         and     7Fh
                jp      z,s7d_sp
                cp      ' '
                jp      z,s7d_sp
                ld      c,a                     ; --- 글자 하나: HMMM
                ld      (s7_cc),a
                and     0Fh
                ld      e,a
                ld      a,c
                cp      10h
                jr      c,s7d_n6
                cp      1Ch
                jr      nc,s7d_n6
                ld      a,e                     ; 아이콘(10h~1Bh): (코드 & 15) x 8, 너비 8
                add     a,a
                add     a,a
                add     a,a
                ld      e,a
                ld      hl,8
                ld      a,1
                jr      s7d_nw
s7d_n6:         ld      a,e
                add     a,a
                add     a,e
                add     a,a                     ; (코드 & 15) x 6, 너비 6
                ld      e,a
                ld      hl,6
                xor     a
s7d_nw:         ld      (s7_icn),a
                ld      (s7_cb+8),hl
                ld      d,0
                ld      hl,(s7_sx0)
                add     hl,de
                ld      (s7_cb),hl
                ld      a,c
                and     0F0h
                rrca                            ; (코드 & F0) / 2 = (코드 >> 4) x 8
                ld      e,a
                ld      d,0
                ld      hl,(s7_sy0)
                add     hl,de
                ld      (s7_cb+2),hl
                ld      hl,(s7_x)
                ld      (s7_cb+4),hl
                ld      de,6
                add     hl,de
                ld      (s7_x),hl
                ld      hl,(s7_tp)
                inc     hl
                ld      (s7_tp),hl
                ld      hl,s7_n
                dec     (hl)
                call    s7_wait
                ld      a,32
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cb
                ld      bc,0F9Bh
                otir
                ld      a,(s7_h)
                or      a
                jp      z,s7d_l
                ld      a,(s7_cc)               ; 7H: 세로선 따위는 위·아래 한 줄씩 이어 그림
                cp      0Ah
                jp      nc,s7d_l
                ld      e,a
                ld      d,0
                ld      hl,hx_tab
                add     hl,de
                ld      a,(hl)
                or      a
                jp      z,s7d_l
                ld      (s7_xf),a
                call    s7d_ext
                jp      s7d_l
s7d_sp:         ld      hl,(s7_tp)
                ld      a,(s7_n)
                ld      b,a
                ld      c,0
s7d_s1:         ld      a,(hl)
                cp      80h
                jr      nc,s7d_s3
                and     7Fh
                jr      z,s7d_s2
                cp      ' '
                jr      nz,s7d_s3
s7d_s2:         inc     c
                inc     hl
                djnz    s7d_s1
s7d_s3:         ld      (s7_tp),hl
                ld      a,b
                ld      (s7_n),a
                ld      l,c
                ld      h,0
                add     hl,hl
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de                   ; 개수 x 6
                ld      (s7_cv+4),hl
                ex      de,hl
                ld      hl,(s7_x)
                ld      (s7_cv),hl
                add     hl,de
                ld      (s7_x),hl
                ld      a,(s7_h)
                or      a
                jp      nz,s7d_l                ; 7H: 빈칸은 미리 칠했으니 건너뜀 (s7_x 만 전진)
                ld      a,(s7_icn)
                or      a
                jr      z,s7d_nz
                xor     a
                ld      (s7_icn),a
                ld      hl,(s7_cv)
                inc     hl
                inc     hl
                ld      (s7_cv),hl
                ld      hl,(s7_cv+4)
                dec     hl
                dec     hl
                ld      (s7_cv+4),hl
s7d_nz:         call    s7_wait
                ld      a,36
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cv
                ld      bc,0B9Bh
                otir
                jp      s7d_l
s7d_hpf:        ld      hl,(s7_x)
                ld      (s7_cv),hl
                ld      hl,12
                ld      (s7_cv+4),hl
                call    s7_wait
                ld      a,36
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cv
                ld      bc,0B9Bh
                otir
                jp      hg_pb                   ; 글자 점 -> 색 표 (이 줄의 색 쌍으로)
s7d_pre:        ld      a,(s7_n)
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      d,h
                ld      e,l
                add     hl,hl
                add     hl,de
                ld      (s7_cv+4),hl
                ld      hl,(s7_x)
                ld      (s7_cv),hl
                call    s7_wait
                ld      a,36
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cv
                ld      bc,0B9Bh
                otir
                jp      hg_pb
s7d_send:       call    s7_wait
                ld      a,32
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cb
                ld      bc,0F9Bh
                otir
                ret
s7d_ext:        ld      a,1
                ld      (s7_cb+10),a
                ld      a,(s7_xf)
                rrca
                jr      nc,sx_nt
                ld      a,(s7_yb)
                ld      (s7_cb+6),a
                call    s7d_send
sx_nt:          ld      a,(s7_xf)
                and     2
                jr      z,sx_nb
                ld      hl,(s7_cb+2)
                ld      de,7
                add     hl,de
                ld      (s7_cb+2),hl
                ld      a,(s7_yb)
                ld      hl,s7_bx
                add     a,(hl)
                ld      (s7_cb+6),a
                call    s7d_send
                ld      hl,(s7_cb+2)
                ld      de,-7
                add     hl,de
                ld      (s7_cb+2),hl
sx_nb:          ld      a,(s7_yb)
                inc     a
                ld      (s7_cb+6),a
                ld      a,8
                ld      (s7_cb+10),a
                ret
; 7H: 한글 쌍 (HL = 글 위치, 첫 바이트 80h 이상).  짝이 없으면 빈칸 한 칸, 있으면 글꼴을 구해 직접 그림
s7d_hg:         ld      a,(s7_n)
                cp      2
                jr      c,s7d_o1
                inc     hl
                ld      a,(hl)
                dec     hl
                cp      80h
                jr      c,s7d_o1
                ld      a,(hl)
                sub     80h
                ld      d,a                     ; idx >> 7
                inc     hl
                ld      a,(hl)
                sub     80h
                ld      e,a                     ; idx AND 7Fh
                inc     hl
                ld      (s7_tp),hl
                ld      hl,s7_n
                dec     (hl)
                dec     (hl)
                ld      h,d
                ld      l,0
                srl     h
                rr      l
                ld      d,0
                add     hl,de                   ; idx
                ld      (hg_idx),hl
                ld      de,HG_NUM
                or      a
                sbc     hl,de
                jr      nc,s7d_o2               ; 범위 밖 = 두 칸 빈칸
                ld      a,(s7_h)
                or      a
                call    z,s7d_hpf               ; (팝업) 12 x 8 칸을 바탕색으로 칠함
                call    s7_wait                 ; 지금까지의 VDP 명령이 끝나야 CPU 가 VRAM 을 씀
                xor     a
                out     (99h),a
                ld      a,8Fh
                out     (99h),a                 ; R#15 = 0 (글꼴 읽기에 DOS 를 부르므로)
                ei
                call    hg_get
                call    hg_draw
                ld      hl,(s7_x)
                ld      de,12
                add     hl,de
                ld      (s7_x),hl
                di
                ld      a,2
                out     (99h),a
                ld      a,8Fh
                out     (99h),a                 ; R#15 = 2
                jp      s7d_l
s7d_o1:         ld      hl,(s7_tp)
                inc     hl
                ld      (s7_tp),hl
                ld      hl,s7_n
                dec     (hl)
                ld      de,6
                jr      s7d_ox
s7d_o2:         ld      de,12
s7d_ox:         ld      hl,(s7_x)
                add     hl,de
                ld      (s7_x),hl
                jp      s7d_l
s7d_end:        call    s7_wait
                xor     a
                out     (99h),a
                ld      a,8Fh
                out     (99h),a                 ; R#15 = 0
                ei
                ret

; ---- (0.12.9) 목록 스크롤 복사.  draw_panel 이 목록을 그리기 전에 부름 (IX = 패널, pn = 0/1).
;      지난번에 그린 top 과 달라졌으면(1 <= 차이 < 목록 줄 수) 화면의 겹치는 목록 줄들을 HMMM 으로 위/아래로 복사하고
;      그 줄들의 해시도 같이 밀어서, 뒤따르는 목록 그리기가 새로 나타난 줄(과 색이 바뀐 줄)만 그리게 함.
;      (해시 = "그 줄 화면이 이 글·색" 이라는 약속이 복사 뒤에도 유지되므로 항상 안전)
s7_lscr:        ld      a,(s7_on)
                or      a
                ret     z
                ld      a,(pn)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,dp_ptop
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 지난번 top
                ld      c,(ix+PS_TOP)
                ld      b,(ix+PS_TOP+1)         ; BC = 새 top
                ld      (hl),b
                dec     hl
                ld      (hl),c
                ld      h,b
                ld      l,c
                or      a
                sbc     hl,de                   ; HL = 새 - 지난
                ret     z
                xor     a
                ld      (sl_dir),a
                ld      a,h
                or      a
                jr      z,sl_pos
                inc     a
                ret     nz
                ld      a,l                     ; 음수 (목록이 아래로 밀림): k = -l
                neg
                ld      l,a
                ld      a,1
                ld      (sl_dir),a
sl_pos:         ld      a,l
                or      a
                ret     z
                ld      hl,v_lrows
                cp      (hl)
                ret     nc                      ; 한 화면 이상 건너뜀: 복사 없음
                ld      (sl_k),a
                ld      a,(s7_h)
                or      a
                jr      z,sl_8
                ld      a,(s7_geo)              ; (0.12.31) 얇은 선 배치의 목록 줄 높이
                ld      e,a
                ld      a,(s7_yh)
                ld      d,a
                jr      sl_g
sl_8:           ld      e,8
                ld      d,S7_Y0
sl_g:           ld      a,(v_lrows)
                ld      hl,sl_k
                sub     (hl)
                ld      b,a                     ; B = 복사할 줄 수 m
                xor     a
sl_m:           add     a,e
                djnz    sl_m
                ld      (s7_sc+10),a            ; NY = m x 줄 높이
                ld      c,a
                ld      a,(sl_k)
                ld      b,a
                xor     a
sl_kh:          add     a,e
                djnz    sl_kh
                ld      b,a                     ; B = k x 줄 높이
                ld      a,e
                add     a,a
                add     a,d
                ld      d,a                     ; D = 목록 첫 줄 y
                ld      a,(s7_th)
                or      a
                jr      z,sl_nt
                ld      a,(ytab+LIST_Y)
                ld      d,a
sl_nt:
                ld      a,(sl_dir)
                or      a
                jr      nz,sl_dn
                ld      a,d                     ; 위로: SY = 첫 줄 + k 줄, DY = 첫 줄, 위에서 아래로 복사
                add     a,b
                ld      (s7_sc+2),a
                ld      a,d
                ld      (s7_sc+6),a
                xor     a
                jr      sl_go
sl_dn:          ld      a,d                     ; 아래로: 맨 아래 줄부터 (DIY = 1)
                add     a,c
                dec     a
                ld      (s7_sc+2),a
                add     a,b
                ld      (s7_sc+6),a
                ld      a,8
sl_go:          ld      (s7_sc+13),a
                ld      hl,S7_X0
                ld      a,(pn)
                or      a
                jr      z,sl_x
                ld      hl,S7_X0+240
sl_x:           ld      (s7_sc),hl
                ld      (s7_sc+4),hl
                ld      hl,PANW*6
                ld      (s7_sc+8),hl
                di
                ld      a,2
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                call    s7_wait
                ld      a,32
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_sc
                ld      bc,0F9Bh
                otir
                call    s7_wait
                xor     a
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                ei
; 해시 밀기: 위로 = 첫 줄부터 +4k 칸에서 가져옴, 아래로 = 끝 줄부터 -4k 칸에서 (k 줄이 안 겹치는 쪽은 비움)
                ld      a,(pn)
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_hash+LIST_Y*4
                add     hl,de
                ld      a,(sl_k)
                add     a,a
                add     a,a
                ld      e,a
                ld      d,0
                ld      (sl_off),de
                ld      de,4
                ld      (sl_stp),de
                ld      a,(sl_dir)
                or      a
                jr      z,sl_h0
                ld      hl,0
                ld      de,(sl_off)
                or      a
                sbc     hl,de
                ld      (sl_off),hl             ; -4k
                ld      hl,-4
                ld      (sl_stp),hl
                ld      a,(v_lrows)
                dec     a
                add     a,a
                add     a,a
                ld      e,a
                ld      d,0
                ld      a,(pn)
                add     a,a
                ld      c,a
                ld      b,0
                ld      hl,s7_hash+LIST_Y*4
                add     hl,de
                add     hl,bc                   ; HL = 끝 줄의 이 패널 해시 칸
sl_h0:          ld      a,(v_lrows)
                ld      b,a
sl_hl:          ld      a,(sl_k)
                cp      b
                jr      nc,sl_hi
                push    hl
                ld      de,(sl_off)
                add     hl,de
                ld      a,(hl)
                inc     hl
                ld      c,(hl)
                pop     hl
                ld      (hl),a
                inc     hl
                ld      (hl),c
                dec     hl
                jr      sl_hn
sl_hi:          ld      (hl),0FFh
                inc     hl
                ld      (hl),0FFh
                dec     hl
sl_hn:          ld      de,(sl_stp)
                add     hl,de
                djnz    sl_hl
                ret
sl_dir:         db      0
sl_k:           db      0
sl_off:         dw      0
sl_stp:         dw      0
dp_ptop:        dw      0,0
s7_sc:          dw      0,0,0,0,0,0             ; HMMM (화면 -> 화면): SX SY DX DY NX NY
                db      0,0,0D0h

; ---- (0.12.21) 얇은 선 띠: 선 줄(패널 머리 아래, 목록 아래, 기능키 위)과 패널 사이 세로선을 HMMV 로 그림 ----
s7_rect:        ld      hl,(rc_x)
                ld      (s7_cv),hl
                ld      a,(rc_y)
                ld      (s7_cv+2),a
                xor     a
                ld      (s7_cv+3),a
                ld      hl,(rc_w)
                ld      (s7_cv+4),hl
                ld      a,(rc_h)
                ld      (s7_cv+6),a
                xor     a
                ld      (s7_cv+7),a
                ld      a,(rc_c)
                ld      (s7_cv+8),a
                di
                ld      a,2
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                call    s7_wait
                ld      a,36
                out     (99h),a
                ld      a,91h
                out     (99h),a
                ld      hl,s7_cv
                ld      bc,0B9Bh
                otir
                call    s7_wait
                xor     a
                out     (99h),a
                ld      a,8Fh
                out     (99h),a
                ei
                ret
s7_bands:       ld      a,(s7_on)
                or      a
                ret     z
                ld      a,(s7_th)
                or      a
                ret     z
                call    sb_col
                call    sb_v0                   ; (0.12.25) 세로선 칸을 먼저 바탕으로 지우고, 가로 띠를 그린 뒤 세로선을 맨 위에
                ld      a,(ytab)                ; (0.12.31) 선 띠는 경계마다 s7_geo 의 (선 위치, 띠 높이): 글꼴 셀의 빈 점까지 합쳐 보이는 간격이 1점
                ld      hl,s7_geo+3
                sub     (hl)
                ld      (rc_y),a
                dec     hl
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
                call    sb_hy                   ; 맨 위 선
                ld      a,1
                ld      hl,s7_geo+4
                call    sb_hr                   ; 머리 아래 선
                ld      a,(v_rrule)
                ld      hl,s7_geo+6
                call    sb_hr                   ; 목록 아래 선
                ld      a,(v_rbot)
                ld      hl,s7_geo+8
                call    sb_hr                   ; 기능키 위 선
                call    sb_y0                   ; 가운데 세로선: 맨 위 선 ~ 목록 아래 선 (x 252)
                ld      b,a
                call    sb_yr
                ld      c,a
                ld      hl,S7_X0+39*6+2
                ld      (rc_x),hl
                call    sb_seg
                xor     a
                jp      sb_edge                 ; 좌우 테두리선
; 세로선 칸 39 (x 250~255) 를 바탕색으로 지움 (맨 위 선 띠 ~ 목록 아래 선 띠 끝)
sb_v0:          ld      a,(ytab)
                ld      hl,s7_geo+3
                sub     (hl)
                ld      (rc_y),a
                ld      a,(v_rrule)
                ld      e,a
                ld      d,0
                ld      hl,ytab
                add     hl,de
                ld      a,(hl)
                ld      hl,s7_geo+7
                add     a,(hl)
                ld      hl,rc_y
                sub     (hl)
                ld      (rc_h),a
                ld      hl,S7_X0+39*6
                ld      (rc_x),hl
                ld      hl,6
                ld      (rc_w),hl
                ld      a,(rc_bg)
                ld      (rc_c),a
                jp      s7_rect
; 색: rc_bg = 바탕(쌍 0 의 둘째), rc_fg = 선(첫째)
sb_col:         ld      a,(s7_pairs+1)
                ld      c,a
                rlca
                rlca
                rlca
                rlca
                or      c
                ld      (rc_bg),a
                ld      a,(s7_pairs)
                ld      c,a
                rlca
                rlca
                rlca
                rlca
                or      c
                ld      (rc_fg),a
                ret
; 선의 y: sb_y0 = 맨 위 선, sb_yr = 목록 아래 선, sb_yb = 기능키 위 선 (A 로 돌려줌)
sb_y0:          ld      a,(ytab)
                ld      hl,s7_geo+3
                sub     (hl)
                ld      hl,s7_geo+2
                add     a,(hl)
                ret
sb_yr:          ld      a,(v_rrule)
                ld      hl,s7_geo+6
                jr      sb_yx
sb_yb:          ld      a,(v_rbot)
                ld      hl,s7_geo+8
sb_yx:          ld      e,a
                ld      d,0
                push    hl
                ld      hl,ytab
                add     hl,de
                ld      a,(hl)
                pop     hl
                add     a,(hl)
                ret
; (0.12.28) 좌우 테두리선(x 14 와 491, 1점).  A = 0: 맨 위 선부터 기능키 위 선까지, 그 밖: 목록 아래 선부터 (정보 줄을 다시 쓴 뒤
;  칸 79 에 걸친 오른쪽 선을 되살림)
sb_edge:        or      a
                jr      nz,se_i
                call    sb_y0
                jr      se_a
se_i:           call    sb_yr
se_a:           ld      b,a
                call    sb_yb
                ld      c,a
                ld      hl,S7_X0-2
                ld      (rc_x),hl
                push    bc
                call    sb_seg
                pop     bc
                ld      hl,S7_X0+79*6+1
                ld      (rc_x),hl
; B = 시작 y, C = 끝 y (그 점까지 포함), rc_x = x → 1점 세로선 (LMMV)
sb_seg:         ld      a,b
                ld      (rc_y),a
                ld      a,c
                sub     b
                inc     a
                ld      (rc_h),a
sb_vl:          ld      hl,1
                ld      (rc_w),hl
                ld      a,(rc_fg)
                ld      (rc_c),a
                ld      a,80h                   ; LMMV (1점 폭)
                ld      (s7_cv+10),a
                call    s7_rect
                ld      a,0C0h
                ld      (s7_cv+10),a
                ret
; 정보 줄을 다시 쓴 뒤: 오른쪽 테두리선(칸 79 에 걸침)을 되살림
s7_bdinfo:      ld      a,(s7_on)
                or      a
                ret     z
                ld      a,(s7_th)
                or      a
                ret     z
                call    sb_col
                ld      a,1
                jp      sb_edge
; A = 선 줄 번호, HL = s7_geo 의 (선 위치, 띠 높이) → 그 줄의 띠를 그림
sb_hr:          ld      e,a
                ld      d,0
                push    hl
                ld      hl,ytab
                add     hl,de
                ld      a,(hl)
                ld      (rc_y),a
                pop     hl
                ld      c,(hl)
                inc     hl
                ld      b,(hl)
; rc_y = 띠 맨 위 y, B = 띠 높이, C = 선 위치(띠 맨 위에서 몇 점 아래): x 14~495 를 바탕으로 칠하고 x 14~491 에 선
sb_hy:          ld      hl,S7_X0-2              ; (0.12.28) 왼쪽 테두리선 x 14 부터
                ld      (rc_x),hl
                ld      hl,482                  ; x 14~495 (칸 79 끝까지 바탕)
                ld      (rc_w),hl
                ld      a,b
                ld      (rc_h),a
                ld      a,(rc_bg)
                ld      (rc_c),a
                push    bc
                call    s7_rect
                pop     bc
                ld      a,(rc_y)
                add     a,c
                ld      (rc_y),a
                ld      a,1
                ld      (rc_h),a
                ld      hl,478                  ; 선: x 14~491 (오른쪽 테두리선까지)
                ld      (rc_w),hl
                ld      a,(rc_fg)
                ld      (rc_c),a
                jp      s7_rect
rc_x:           dw      0
rc_w:           dw      0
rc_y:           db      0
rc_h:           db      0
rc_c:           db      0
rc_bg:          db      0
rc_fg:          db      0

; ---- 해시 칸: 반쪽 줄(39 글자 x 2) 또는 한 줄(80 글자)이면 A=1, HL=칸 주소
;      아니면 겹치는 칸을 무효로 하고 A=0
s7_hslot:       ld      hl,0
                ld      (s7_hx),hl
                ld      a,(s7_row)
                add     a,a
                add     a,a
                ld      e,a
                ld      d,0
                ld      hl,s7_hash
                add     hl,de
                ld      a,(s7_n)
                ld      b,a
                ld      a,(s7_col)
                ld      c,a
                ld      a,b
                cp      39
                jr      nz,s7h_1
                ld      a,c
                or      a
                jr      nz,s7h_2
                ld      a,1
                ret
s7h_2:          cp      40
                jr      nz,s7h_x
                inc     hl
                inc     hl
                ld      a,1
                ret
s7h_1:          cp      80
                jr      nz,s7h_x
                ld      a,c
                or      a
                jr      nz,s7h_x
                push    hl
                inc     hl
                inc     hl
                ld      (s7_hx),hl
                pop     hl
                ld      a,1
                ret
s7h_x:          ld      a,c
                cp      39
                jr      nc,s7h_r
                ld      (hl),0FFh
                inc     hl
                ld      (hl),0FFh
                dec     hl
s7h_r:          ld      a,c
                add     a,b
                dec     a
                cp      40
                jr      c,s7h_e
                inc     hl
                inc     hl
                ld      (hl),0FFh
                inc     hl
                ld      (hl),0FFh
s7h_e:          xor     a
                ret
; ---- 글 해시 (16비트)  → HL
s7_hh:          ld      de,(s7_tp)
                ld      a,(s7_pair)
                ld      h,a
                ld      a,(s7_n)
                ld      l,a
                ld      b,a
s7hh_l:         add     hl,hl
                jr      nc,s7hh_a
                inc     l
s7hh_a:         ld      a,(de)
                inc     de
                add     a,l
                ld      l,a
                jr      nc,s7hh_b
                inc     h
s7hh_b:         djnz    s7hh_l
                ld      a,h
                and     l
                inc     a
                ret     nz
                dec     hl
                ret

; ---- 목록 줄 하나의 색·아이콘 정함 (draw_panel 이 put_row 앞에서 부름).  C = FFh 이면 커서 줄
s7_prep:        ld      a,(s7_on)
                or      a
                ret     z
                ld      a,(s7_ent)
                or      a
                ret     z
                push    bc
                ld      hl,(en_p)
                push    hl
                pop     iy
                call    is_dotdot
                ld      a,18h                   ; ".." = 위 화살표
                ld      b,0
                jr      z,s7p_ic
                ld      a,(iy+EN_ATTR)
                and     10h
                ld      b,1                     ; 폴더
                jr      nz,s7p_t
                ld      b,0
                ld      de,s7_ext
s7p_e:          ld      a,(de)
                or      a
                jr      z,s7p_t
                ld      h,d
                ld      l,e
                ld      a,(iy+EN_EXT)
                cp      (hl)
                jr      nz,s7p_n
                inc     hl
                ld      a,(iy+EN_EXT+1)
                cp      (hl)
                jr      nz,s7p_n
                inc     hl
                ld      a,(iy+EN_EXT+2)
                cp      (hl)
                jr      nz,s7p_n
                inc     hl
                ld      b,(hl)
                jr      s7p_t
s7p_n:          inc     de
                inc     de
                inc     de
                inc     de
                jr      s7p_e
s7p_t:          ld      hl,s7_icmap
                ld      e,b
                ld      d,0
                add     hl,de
                ld      a,(hl)
s7p_ic:         push    bc                      ; 한 칸 오른쪽으로 밀고 첫 칸 = 아이콘, 둘째 칸 = 빈칸
                push    af
                ld      hl,linebuf+37
                ld      de,linebuf+38
                ld      bc,38
                lddr
                pop     af
                ld      (linebuf),a
                ld      a,' '
                ld      (linebuf+1),a
                pop     bc
                call    s7_uext                 ; 색 쌍 = 형 (INI 의 COLOR 줄이 있으면 그 색)
                bit     0,(iy+EN_FLAG)
                jr      z,s7p_s
                ld      a,10                    ; 고름
s7p_s:          pop     bc
                push    af
                ld      a,c
                or      a
                jr      z,s7p_x
                pop     af
                cp      10
                ld      a,9                     ; 커서
                jr      c,s7p_c
                ld      a,11                    ; 고름 + 커서
s7p_c:          ld      (s7_pair),a
                ret
s7p_x:          pop     af
                ld      (s7_pair),a
                ret

; ======================================================================
; ESC = 처음 DOS 화면 보기 (다시 ESC 로 돌아옴).  시작할 때 글자 화면의 이름표(글자)를 VRAM 에 맡겨 둠:
;   글자 화면일 때 4000h, SCREEN 7 일 때 D400h(쪽 0 의 화면 아래 줄 212~255 = 쓰지 않는 곳).
;   옮길 때는 SECBUF 를 거침 (잠깐 쓰는 곳).
; ======================================================================
S7_DOSV         equ     0D400h
; HL = VRAM 주소(0~FFFFh), BC = 길이, A = 0 읽기(VRAM → SECBUF) / 1 쓰기(SECBUF → VRAM)
dos_io:         ld      (dio_n),bc
                ld      (dio_m),a
                push    hl
                ld      a,h
                rlca
                rlca
                and     3
                ld      b,14
                call    vdp_wreg                ; R#14 = 주소 윗부분
                pop     hl
                ld      a,(dio_m)
                or      a
                ld      c,0
                jr      z,dio_a
                ld      c,40h
dio_a:          di
                ld      a,l
                out     (99h),a
                ld      a,h
                and     3Fh
                or      c
                out     (99h),a
                ei
                ld      hl,SECBUF
                ld      bc,(dio_n)
                ld      a,(dio_m)
                or      a
                jr      nz,dio_w
dio_r:          in      a,(98h)
                ld      (hl),a
                inc     hl
                dec     bc
                ld      a,b
                or      c
                jr      nz,dio_r
                jr      dio_e
dio_w:          ld      a,(hl)
                out     (98h),a
                inc     hl
                dec     bc
                ld      a,b
                or      c
                jr      nz,dio_w
dio_e:          xor     a
                ld      b,14
                jp      vdp_wreg
; 시작할 때: 지금이 글자 화면이면 그 글자를 읽어 4000h 에 맡김
dos_grab:       ld      hl,(0F3DCh)             ; 커서 자리 (CSRY, CSRX)
                ld      (dos_cur),hl
                xor     a
                ld      (dos_ok),a
                ld      a,(SCRMOD)
                or      a
                ret     nz
                ld      a,(CRTCNT)
                or      a
                ret     z
                ld      b,a
                ld      a,(LINL40)
                ld      e,a
                ld      d,0
                ld      hl,0
dg_m:           add     hl,de
                djnz    dg_m
                ld      a,h
                cp      8
                ret     nc                      ; 2048 넘으면 안 맡김
                ld      (dos_len),hl
                ld      b,h
                ld      c,l
                ld      hl,0
                xor     a
                call    dos_io                  ; 이름표 → SECBUF
                ld      a,1
                ld      (dos_ok),a
                jr      dos_st0
dos_stash0:     ld      a,(dos_ok)              ; SECBUF → 4000h
                or      a
                ret     z
dos_st0:        ld      hl,4000h
                ld      bc,(dos_len)
                ld      a,1
                jp      dos_io
dos_stash7:     ld      a,(dos_ok)              ; SECBUF → D400h
                or      a
                ret     z
                ld      hl,S7_DOSV
                ld      bc,(dos_len)
                ld      a,1
                jp      dos_io
dos_fetch:      ld      a,(dos_ok)              ; 맡겨 둔 곳 → SECBUF
                or      a
                ret     z
                ld      hl,4000h
                ld      a,(s7_on)
                or      a
                jr      z,df_g
                ld      hl,S7_DOSV
df_g:           ld      bc,(dos_len)
                xor     a
                jp      dos_io
; SECBUF 의 DOS 글을 이름표에 되돌림 (맡긴 게 없으면 아무것도 안 함)
dos_wr:         ld      a,(dos_ok)
                or      a
                ret     z
                ld      hl,0
                ld      bc,(dos_len)
                ld      a,1
                call    dos_io
                ld      hl,(dos_cur)            ; 커서 자리도 처음대로 (CSRY, CSRX)
                ld      (0F3DCh),hl
                ret
; ESC: DOS 화면을 보여 줌
ml_dos:         ld      a,(dos_ok)
                or      a
                jp      z,ml_next
                call    dos_fetch
                call    s7_leave
                ld      a,(old_linl)
                ld      (LINL40),a
                ld      a,(old_scrmod)
                ld      ix,CHGMOD
                call    bios
                call    clr_nt
                ld      hl,0
                ld      bc,(dos_len)
                ld      a,1
                call    dos_io                  ; 글을 이름표에 되돌림
dd_w:           call    wait_key
                cp      1Bh
                jr      nz,dd_w
                call    scr_text
                call    dos_stash0              ; SECBUF 는 그대로 → 맡김 자리
                ld      a,(s7_want)
                or      a
                call    nz,s7_enter
                call    draw_all
                jp      ml_next
dos_ok:         db      0
dos_rest:       db      0
dos_cur:        dw      0
dos_len:        dw      0
dio_n:          dw      0
dio_m:          db      0

; ---- 자료
s7_icn:         db      0                       ; 방금 그린 것이 아이콘(8점 폭)
s7_on:          db      0                      ; 지금 SCREEN 7 로 그리는 중
s7_want:        db      0                       ; 쓰고 싶음 (프로그램을 다녀와도 유지)
s7_pair:        db      0
s7_ent:         db      0
s7_row:         db      0
s7_col:         db      0
s7_n:           db      0
s7_tp:          dw      0
s7_x:           dw      0
s7_sx0:         dw      0
s7_sy0:         dw      0
s7_hx:          dw      0
s7b_p:          db      0
s7b_n:          db      0
s7b_xb:         db      0
s7b_y:          dw      0
s7_pb:          ds      4
s7_lb:          ds      48
s7_xo:          dw      0
s7_hash:        ds      112                     ; 28줄 x 4
s7_cb:          dw      0,0,0,0,6,8             ; HMMM: SX SY DX DY NX NY
                db      0,0,0D0h                ;       CLR ARG CMD
s7_cv:          dw      0,0,0,8                 ; HMMV: DX DY NX NY
                db      0,0,0C0h                ;       CLR ARG CMD
s7_cvall:       dw      0,0,512,212
                db      11h,0,0C0h
; 색 쌍 (글자, 바탕): 0 기본, 1 폴더, 2 프로그램, 3 배치, 4 글, 5 그림, 6 음악, 7 압축, 8 롬, 9 커서, 10 고름, 11 고름+커서,
;   12 팝업, 13~19 = INI 의 COLOR 줄 색 (바탕 1)
s7_pairs:       db      4,1, 8,1, 9,1, 15,1, 10,1, 11,1, 12,1, 13,1, 14,1, 4,6, 7,1, 7,6
                db      4,2, 4,1, 4,1, 4,1, 4,1, 4,1, 4,1, 4,1
s7_np:          db      13                      ; 만들 글꼴판 수 (13 + INI 색 수)
s7_sxt:         dw      0,96,192,288,384, 0,96,192,288,384, 0,96,192,288,384, 0,96,192,288,384
s7_syt:         dw      256,256,256,256,256, 320,320,320,320,320, 384,384,384,384,384, 448,448,448,448,448

; ---- INI 읽기 (한 번): "SCREEN = 7" 이면 SCREEN 7 로 시작, "COLOR 확장자 = 번호(0~15)" = 그 확장자 색
; 시작할 때: INI 를 읽고, 명령줄 /7 · /0 이 있으면 그 방식으로 시작하고 INI 의 SCREEN 줄에 저장(다음부터도 유지)
s7_ini:         call    s7_ini0
                ld      a,(s7_want)
                cp      2
                jr      nz,s7ini_h
                call    hgl_open                ; INI·옵션이 7H 인데 X86FC.HGL 이 없으면 SCREEN 7 으로
                jr      z,s7ini_h
                ld      a,1
                ld      (s7_want),a
s7ini_h:        jp      h_setmode
s7_ini0:        call    s7_ini1
                ld      a,(s7_sortset)          ; INI 의 SORT 가 있으면 두 패널을 다시 정렬
                or      a
                jr      z,s7ini_c
                ld      ix,pstate
                call    resort_panel
                ld      ix,pstate+PSZ
                call    resort_panel
s7ini_c:        ld      a,(0080h)
                or      a
                ret     z
                ld      b,a
                ld      hl,0081h
s7o_l:          ld      a,(hl)
                cp      '/'
                jr      z,s7o_s
                inc     hl
                djnz    s7o_l
                ret
s7o_s:          inc     hl
                ld      a,(hl)
                cp      '7'
                jr      z,s7o_g
                cp      '0'
                ret     nz
s7o_g:          ld      (si_ch),a
                xor     a
                ld      (si_c2),a
                inc     hl
                ld      a,(hl)
                dec     hl
                and     0DFh
                cp      'H'                     ; /7H
                ld      a,(hl)
                jr      nz,s7o_n
                cp      '7'
                jr      nz,s7o_n
                ld      a,'H'
                ld      (si_c2),a
                ld      a,2
                jr      s7o_w
s7o_n:          cp      '7'
                ld      a,0
                jr      nz,s7o_w
                inc     a
s7o_w:          ld      (s7_want),a
                ld      hl,s7_kscr
                ld      (si_kwp),hl
                ld      hl,s7_stxt
                ld      (si_txp),hl
                ld      hl,9
                ld      (si_txn),hl
; INI 의 한 키 줄(si_kwp 로 시작하는 줄)을 지우고 끝에 "키 = <si_ch>" (si_txp, 길이 si_txn) 를 붙여 다시 저장 (다른 줄은 그대로)
s7_setini:      ld      hl,(si_kwp)             ; (0.12.14) INI 의 그 키 줄을 지우고 새로 붙임 (SET 파일 폐지)
                ld      (iw_key),hl
                ld      hl,iw_mkey
                ld      de,gen_set
                jp      ini_wr
gen_set:        ld      de,CMDBUF
                ld      hl,(si_txp)
                ld      bc,(si_txn)
                ldir
                ld      a,(si_ch)
                ld      (de),a
                inc     de
                ld      a,(si_c2)
                or      a
                jr      z,gs_n
                ld      (de),a
                inc     de
gs_n:           xor     a
                ld      (de),a
                ld      hl,CMDBUF
                jp      iw_put
s7_stxt:        db      "SCREEN = "
si_ch:          db      '7'
si_c2:          db      0
s7_ini1:        call    il_open                 ; (0.12.14) INI 전체를 줄 단위로 읽음 (SET 파일 폐지)
                ret     nz
s7i_nl:         call    il_next
                jp      z,s7i_done
                ld      hl,ILBUF
s7i_ln:         ld      a,(hl)
                or      a
                jr      z,s7i_nl
                cp      21h
                jr      nc,s7i_go
                inc     hl
                jr      s7i_ln
s7i_go:         ld      de,s7_kcol
                push    hl
                call    s7i_kw
                jp      z,s7i_c
                pop     hl
                push    hl
                ld      de,s7_kscr
                call    s7i_kw
                jp      z,s7i_s
                pop     hl
                push    hl
                ld      de,s7_ksrt
                call    s7i_kw
                jp      z,s7i_so
                pop     hl
                push    hl
                ld      de,s7_kbgp
                call    s7i_kw
                jp      z,s7i_bg
                pop     hl
                push    hl
                ld      de,s7_klin
                call    s7i_kw
                jp      z,s7i_li
s7i_sk:         pop     hl
s7i_sk2:        jr      s7i_nl
s7i_done:       jp      il_close
s7i_s:          pop     de                      ; (HL = 키워드 뒤) = 번호 7
                call    s7i_eq
                cp      '7'
                jr      z,s7i_s7
                cp      '1'                     ; (0.11.2 까지 F4 가 잘못 적은 "SCREEN = 1" 도 7 로 읽음)
                jr      nz,s7i_sk3
s7i_s7:         inc     hl
                ld      a,(hl)
                and     0DFh
                cp      'H'                     ; SCREEN = 7H
                ld      a,1
                jr      nz,s7i_sw
                inc     a
s7i_sw:         ld      (s7_want),a
s7i_sk3:        jr      s7i_sk2
s7i_c:          pop     de                      ; COLOR 확장자 = 번호
                ld      a,(hl)
                cp      ' '
                jr      nz,s7i_sk2
s7i_c1:         inc     hl
                ld      a,(hl)
                cp      ' '
                jr      z,s7i_c1
                ld      de,s7_tx                ; 확장자 3 글자 (대문자, 공백 채움)
                ld      a,' '
                ld      (de),a
                inc     de
                ld      (de),a
                inc     de
                ld      (de),a
                ld      de,s7_tx
                ld      b,3
s7i_c2:         ld      a,(hl)
                cp      ' '
                jr      z,s7i_c3
                cp      '='
                jr      z,s7i_c3
                cp      21h
                jr      c,s7i_sk2
                cp      'a'
                jr      c,s7i_c4
                and     0DFh
s7i_c4:         ld      (de),a
                inc     de
                inc     hl
                djnz    s7i_c2
s7i_c3:         call    s7i_eq                  ; A = 번호 첫 글자
                sub     '0'
                cp      10
                jr      nc,s7i_sk2
                ld      c,a
                inc     hl
                ld      a,(hl)
                sub     '0'
                cp      10
                jr      nc,s7i_c5
                ld      b,a
                ld      a,c
                add     a,a
                ld      c,a
                add     a,a
                add     a,a
                add     a,c
                add     a,b                     ; (10 x 앞) + 뒤  (c = 앞 x 2, a = 앞 x 8 + 앞 x 2)
                ld      c,a
s7i_c5:         ld      a,c
                and     0Fh
                ld      (s7_tc),a               ; 색 번호
                ld      (s7_ip),hl              ; 읽던 자리
                ld      a,(s7_nux)
                cp      16
                jr      nc,s7i_end
                ld      a,(s7_np)
                ld      b,a
                ld      c,13                    ; 같은 색이 이미 있나 (13+k 쌍)
s7i_f:          ld      a,c
                cp      b
                jr      nc,s7i_new
                call    s7i_pa
                ld      a,(s7_tc)
                cp      (hl)
                jr      z,s7i_add
                inc     c
                jr      s7i_f
s7i_new:        ld      a,c
                cp      20
                jr      nc,s7i_end              ; 색 가짓수는 7 개까지
                call    s7i_pa
                ld      a,(s7_tc)
                ld      (hl),a
                ld      a,c
                inc     a
                ld      (s7_np),a
s7i_add:        ld      a,(s7_nux)              ; 항목 = 확장자 3 글자 + 쌍 번호
                add     a,a
                add     a,a
                ld      l,a
                ld      h,0
                ld      de,s7_ux
                add     hl,de
                ex      de,hl
                ld      hl,s7_tx
                ld      a,c
                ld      bc,3
                ldir
                ld      (de),a
                ld      hl,s7_nux
                inc     (hl)
s7i_end:        ld      hl,(s7_ip)
                jp      s7i_sk2
s7i_pa:         ld      l,c                     ; C = 쌍 번호 → HL = 그 글자색 칸
                ld      h,0
                add     hl,hl
                ld      de,s7_pairs
                add     hl,de
                ret
s7i_so:         pop     de                      ; SORT = name / ext / size / date (첫 글자만 봄)
                call    s7i_eq
                and     0DFh
                ld      b,0
                cp      'N'
                jr      z,s7i_so2
                inc     b
                cp      'E'
                jr      z,s7i_so2
                inc     b
                cp      'S'
                jr      z,s7i_so2
                inc     b
                cp      'D'
                jp      nz,s7i_sk2
s7i_so2:        ld      a,b
                ld      (sort_mode),a
                ld      a,1
                ld      (s7_sortset),a
                jp      s7i_sk2
s7i_li:         pop     de                      ; (0.12.16) LINES = tight: 7H 줄 높이 9점 (23줄)
                call    s7i_eq
                and     0DFh
                ld      b,0
                cp      'T'
                jr      nz,s7i_l2
                inc     b
s7i_l2:         ld      a,b
                ld      (s7_tight),a
                jp      s7i_sk2
s7i_bg:         pop     de                      ; BGP = RGB (각 0~7, 예 215): 바탕색(팔레트 1번)
                call    s7i_eq
                sub     '0'
                cp      8
                jp      nc,s7i_sk2
                ld      c,a                     ; R
                inc     hl
                ld      a,(hl)
                sub     '0'
                cp      8
                jp      nc,s7i_sk2
                ld      d,a                     ; G
                inc     hl
                ld      a,(hl)
                sub     '0'
                cp      8
                jp      nc,s7i_sk2
                ld      b,a                     ; B
                ld      a,c
                rlca
                rlca
                rlca
                rlca
                or      b
                ld      (s7_palette+2),a
                ld      a,d
                ld      (s7_palette+3),a
                jp      s7i_sk2
s7i_kw:         ld      a,(de)                  ; HL 이 소문자 키워드(DE)로 시작하나 → Z = 맞음 (HL 전진)
                or      a
                ret     z
                ld      a,(hl)
                or      20h
                ld      c,a
                ld      a,(de)
                cp      c
                ret     nz
                inc     hl
                inc     de
                jr      s7i_kw
s7i_eq:         ld      a,(hl)                  ; 공백 건너뛰고 '=' 뒤 공백 건너뜀 → A = 그 글자
                cp      ' '
                jr      nz,s7i_e2
                inc     hl
                jr      s7i_eq
s7i_e2:         cp      '='
                ret     nz
                inc     hl
s7i_e3:         ld      a,(hl)
                cp      ' '
                ret     nz
                inc     hl
                jr      s7i_e3
s7_kcol:        db      "color",0
s7_kscr:        db      "screen",0
s7_ksrt:        db      "sort",0
s7_kbgp:        db      "bgp",0
s7_klin:        db      "lines",0
s7_stsrt:       db      "SORT = "
sort_chars:     db      "NESD"
s7_sortset:     db      0
si_kwp:         dw      s7_kscr
si_txp:         dw      s7_stxt
si_txn:         dw      9
; F8 에서 고른 정렬(A = 0 이름 1 확장자 2 크기 3 날짜)을 INI 의 SORT 줄에 저장
ini_sort:       xor     a
                ld      (si_c2),a
                ld      hl,sort_chars
                ld      e,a
                ld      d,0
                add     hl,de
                ld      a,(hl)
                ld      (si_ch),a
                ld      hl,s7_ksrt
                ld      (si_kwp),hl
                ld      hl,s7_stsrt
                ld      (si_txp),hl
                ld      hl,7
                ld      (si_txn),hl
                call    s7_setini
                ld      hl,s7_kscr              ; (다음 /7 /0 을 위해 되돌려 둠)
                ld      (si_kwp),hl
                ld      hl,s7_stxt
                ld      (si_txp),hl
                ld      hl,9
                ld      (si_txn),hl
                ret

; INI 색: 확장자 → 쌍 번호.  (IY = 항목)  B = 기본 형 번호 → A = 쓸 쌍 번호
s7_uext:        ld      a,(s7_nux)
                or      a
                jr      z,su_n
                ld      c,a
                ld      hl,s7_ux
su_l:           ld      d,h
                ld      e,l
                ld      a,(iy+EN_EXT)
                cp      (hl)
                jr      nz,su_x
                inc     hl
                ld      a,(iy+EN_EXT+1)
                cp      (hl)
                jr      nz,su_x
                inc     hl
                ld      a,(iy+EN_EXT+2)
                cp      (hl)
                jr      nz,su_x
                inc     hl
                ld      a,(hl)
                ret
su_x:           ex      de,hl
                inc     hl
                inc     hl
                inc     hl
                inc     hl
                dec     c
                jr      nz,su_l
su_n:           ld      a,b
                ret
s7_nux:         db      0
s7_tc:          db      0
s7_ip:          dw      0
s7_tx:          ds      3
s7_ux:          ds      64
; 형 번호 → 아이콘 글자: 0 파일 1 폴더 2 프로그램 3 배치 4 글 5 그림 6 음악 7 압축 8 롬
s7_icmap:       db      17h,10h,11h,11h,12h,13h,14h,15h,16h
s7_ext:         db      "COM",2, "EXE",2, "BAT",3
                db      "TXT",4, "DOC",4, "INI",4, "CFG",4, "ASM",4, "LST",4, "HLP",4
                db      "SC5",5, "SC7",5, "SC8",5, "SCA",5, "SCC",5, "PIC",5, "BMP",5, "GIF",5, "SR5",5, "SR7",5, "SR8",5
                db      "MID",6, "MBM",6, "MWM",6, "MWK",6, "MPK",6, "KSS",6, "MGS",6, "BGM",6, "OPX",6
                db      "LZH",7, "ZIP",7, "ARJ",7, "LHA",7, "ARC",7, "PMA",7
                db      "ROM",8, "MX1",8, "MX2",8
                db      "A16",8, "GB ",8, "SG ",8, "SMS",8, "SGR",8
                db      "GE5",5, "GE7",5, "GE8",5, "GV ",5, "IL1",5, "IL2",5, "JPG",5, "MAG",5, "MAX",5, "MIF",5, "MIG",5
                db      "MKI",5, "PCX",5, "PNG",5, "PL ",5, "S16",5, "SC2",5, "SC4",5, "SC6",5, "YJK",5, "BM8",5, "BPE",5
                db      "ADX",6, "IMS",6, "MP3",6, "MSD",6, "PCM",6, "SDT",6, "VGM",6, "WAV",6, "WMA",6, "RCP",6, "M3U",6, "MA ",6, "MBK",6
                db      "AS ",4, "ASC",4, "BAS",4, "C  ",4, "CAP",4, "GEN",4, "H  ",4, "INC",4, "ISS",4, "LDR",4, "MCF",4, "MD ",4, "ME ",4
                db      "BZ2",7, "GZ ",7
                db      0
; 팔레트 (RB, G) 16색: 0 짙은 곤색 1 바탕 2 3 선 4 글 5 흐림 6 선택 바탕 7 노랑 8 폴더 9 프로그램 10 글 11 그림 12 음악 13 압축 14 롬 15 배치
s7_palette:     db      01h,00h, 02h,01h, 13h,01h, 24h,02h, 67h,07h, 46h,05h, 15h,03h, 71h,06h
                db      70h,05h, 23h,07h, 47h,06h, 76h,03h, 71h,04h, 63h,04h, 57h,04h, 26h,07h
s7_defpal:      db      00h,00h, 00h,00h, 11h,06h, 33h,07h, 17h,01h, 37h,03h, 51h,01h, 27h,06h
                db      71h,01h, 73h,03h, 61h,06h, 64h,06h, 11h,04h, 65h,02h, 55h,05h, 77h,07h
; 아이콘 12개 (8점 폭, 오른쪽 빈 칸으로 2점 넘침): 폴더 프로그램 글 그림 음악 압축 롬 파일 위 ← → (1Bh 빈 것)
s7_icons:       db      070h,08Fh,081h,081h,081h,081h,0FFh,000h         ; 10h 폴더 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      0FFh,0FFh,081h,0A1h,091h,0ADh,0FFh,000h         ; 11h 프로그램 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      078h,044h,042h,05Ah,042h,07Ah,07Eh,000h         ; 12h 글 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      0FFh,087h,087h,091h,0B9h,0FDh,0FFh,000h         ; 13h 그림 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      03Eh,022h,022h,022h,022h,066h,066h,000h         ; 14h 음악 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      0FFh,0FFh,091h,089h,091h,089h,0FFh,000h         ; 15h 압축 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      07Eh,042h,05Ah,042h,042h,07Eh,03Ch,03Ch         ; 16h 롬 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      078h,044h,042h,042h,042h,042h,07Eh,000h         ; 17h 파일 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      018h,03Ch,07Eh,0FFh,018h,018h,018h,000h         ; 18h 위 폴더 (0.12.20 새 안, 8x8, 비트 7 = 왼쪽 점)
                db      010h,030h,070h,0FFh,0FFh,070h,030h,010h         ; 19h ← (왼쪽 창 선택, 굵은 블록 화살표)
                db      008h,00Ch,00Eh,0FFh,0FFh,00Eh,00Ch,008h         ; 1Ah → (오른쪽 창 선택)
                db      000h,000h,000h,000h,000h,000h,000h,000h         ; 1Bh (빈 것)
