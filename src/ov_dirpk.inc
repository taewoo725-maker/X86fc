; 만든이: KimTaeWoo
; X86FC 압축 풀 폴더 고르기 풀다운 (오버레이 15) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 압축 목록(오버레이 6)에서 Enter·INS 를 누르면 오버레이 13(ar_ext)이 이 칸으로 건너옴 (PROGBUF = 시작 폴더 = 반대 패널 폴더).
;  폴더를 오가며 고름: Up/Down 이동, Left/Right 한 쪽, RETURN = 폴더로 들어감 (맨 위 "<Extract here>" = 이 폴더에 풀기),
;  ".." / BS = 위 폴더, \ = 루트, 글자 = 그 글자로 시작하는 폴더, ESC = 취소.
;  고르면 PROGBUF = 고른 폴더, 오버레이 13 의 진입 1(ar_ext2)로 / 취소하면 오버레이 6 의 진입 2(목록)로 돌아감.
; 목록 모양·키는 본체 파일 선택 창(pick_file)과 같음. 항목 14 바이트(종류 + 이름 13) 를 CPBUF+1040 부터 DKMAX 개
;  (CPBUF 앞쪽 1032 바이트는 압축 목록의 머리 위치 표·표시 비트라 건드리면 안 됨).
; ----------------------------------------------------------------------
DKMAX           equ     70
DKBUF           equ     CPBUF+1040

dk_start:       ld      hl,PROGBUF
                ld      de,dk_dir
                call    copy_z
                xor     a
                ld      (de),a
dk_scan:        call    dk_read
                xor     a
                ld      (dk_sel),a
                ld      (dk_top),a
dk_draw:        ld      de,errbuf               ; 제목 = 안내 + 지금 폴더
                ld      hl,dk_ttl
                call    copy_z
                ld      hl,dk_dir
                call    copy_z
                xor     a
                ld      (de),a
                xor     a
                ld      (dk_r),a
                ld      a,14
                ld      (dk_re),a               ; 처음·폴더가 바뀔 때만 14 줄 모두 그림
                ld      a,(dk_sel)
                ld      (dk_old),a
                call    dkd_l
                jp      dk_key
dkd_l:          ld      hl,qd_tmp               ; 한 줄(60 칸) 바탕
                ld      a,(dk_r)
                or      a
                jr      nz,dkd_n0
                ld      c,CH_TL
                ld      d,CH_HBAR
                ld      e,CH_TR
                jr      dkd_f
dkd_n0:         cp      13
                jr      nz,dkd_n5
                ld      c,CH_BL
                ld      d,CH_HBAR
                ld      e,CH_BR
                jr      dkd_f
dkd_n5:         ld      c,CH_VBAR
                ld      d,' '
                ld      e,CH_VBAR
dkd_f:          ld      (hl),c
                inc     hl
                ld      b,58
dkd_fl:         ld      (hl),d
                inc     hl
                djnz    dkd_fl
                ld      (hl),e
                ld      a,(dk_r)
                cp      1
                jr      nz,dkd_x2
                ld      hl,errbuf
                jr      dkd_txt
dkd_x2:         cp      12
                jr      nz,dkd_x3
                ld      hl,dk_hint
                jr      dkd_txt
dkd_x3:         cp      2
                jr      c,dkd_wr
                cp      13
                jr      z,dkd_wr                ; 맨 아래 테두리 줄에는 항목 글을 쓰지 않음
                sub     2
                ld      b,a                     ; 목록 줄 번호 0..9
                ld      a,(dk_top)
                add     a,b
                ld      (dk_i),a
                ld      b,a
                ld      a,(dk_n)
                cp      b
                jr      z,dkd_wr
                jr      c,dkd_wr
                ld      a,(dk_i)
                call    dk_adr
                ld      a,(hl)
                ld      (dk_at),a
                inc     hl
                cp      0FFh
                jr      nz,dkd_nm
                ld      hl,dk_okt               ; 맨 위 항목 = "<Extract here>"
dkd_nm:         ld      de,qd_tmp+5
                ld      b,50
dkd_nc:         ld      a,(hl)
                or      a
                jr      z,dkd_ne
                ld      (de),a
                inc     hl
                inc     de
                djnz    dkd_nc
dkd_ne:         ld      a,(dk_at)
                cp      10h
                jr      nz,dkd_mk0
                ld      a,'\'
                ld      (de),a
dkd_mk0:        ld      hl,dk_sel
                ld      a,(dk_i)
                cp      (hl)
                ld      a,' '
                jr      nz,dkd_mk
                ld      a,'>'
dkd_mk:         ld      (qd_tmp+3),a
                jr      dkd_wr
dkd_txt:        ld      de,qd_tmp+3             ; HL 글을 54 칸 안에서 복사
                ld      b,54
dkd_tc:         ld      a,(hl)
                or      a
                jr      z,dkd_wr
                ld      (de),a
                inc     hl
                inc     de
                djnz    dkd_tc
dkd_wr:         ld      a,(dk_r)
                add     a,5
                call    row_addr
                ld      de,10
                add     hl,de
                ld      de,qd_tmp
                ld      b,60
                ld      a,12                    ; (SCREEN 7) 팝업 색
                ld      (s7_pair),a
                call    scr_writep
                xor     a
                ld      (s7_pair),a
                ld      a,(dk_r)
                inc     a
                ld      (dk_r),a
                ld      hl,dk_re
                cp      (hl)
                jp      c,dkd_l
                ret
dk_key:         call    wait_key
                cp      1Bh
                jr      z,dk_esc
                cp      K_DOWN
                jr      z,dk_dn
                cp      K_UP
                jr      z,dk_up
                cp      K_RIGHT
                jp      z,dk_pd
                cp      K_LEFT
                jp      z,dk_pu
                cp      0Dh
                jp      z,dk_ent
                cp      08h
                jp      z,dk_par
                cp      '\'
                jp      z,dk_root
                cp      K_F7                    ; F7 / Ctrl+F = 이 폴더 안에 새 폴더 만들기
                jp      z,dk_mk
                cp      06h
                jp      z,dk_mk
                cp      20h
                jr      c,dk_key
                jp      dk_ta
dk_esc:         pop     hl                      ; 취소: 압축 목록(오버레이 6)으로, 목록을 다시 그림
                ld      a,6
                ld      b,2
                jp      ov_go
dk_dn:          ld      a,(dk_sel)
                inc     a
                ld      hl,dk_n
                cp      (hl)
                jr      nc,dk_key
                ld      (dk_sel),a
                jr      dk_scr
dk_up:          ld      a,(dk_sel)
                or      a
                jr      z,dk_key
                dec     a
                ld      (dk_sel),a
dk_scr:         ld      a,(dk_top)
                ld      (dk_ot),a
                ld      a,(dk_sel)              ; 화면 밖이면 맞춤
                ld      hl,dk_top
                cp      (hl)
                jr      nc,dk_s2
                ld      (hl),a
dk_s2:          ld      a,(dk_top)
                add     a,10
                ld      b,a
                ld      a,(dk_sel)
                cp      b
                jr      c,dk_upd
                ld      a,(dk_sel)
                sub     9
                ld      (dk_top),a
dk_upd:         ld      a,(dk_ot)               ; 목록이 넘어가면 목록 10 줄만, 아니면 예전·새 줄 두 줄만
                ld      hl,dk_top
                cp      (hl)
                jr      z,dk_u1
                ld      a,2
                ld      (dk_r),a
                ld      a,12
                ld      (dk_re),a
                call    dkd_l
                jr      dk_u2
dk_u1:          ld      a,(dk_old)
                call    dk_row
                ld      a,(dk_sel)
                call    dk_row
dk_u2:          ld      a,(dk_sel)
                ld      (dk_old),a
                jp      dk_key
dk_row:         ld      hl,dk_top               ; A = 항목 번호 → 그 줄만 그림
                sub     (hl)
                add     a,2
                ld      (dk_r),a
                inc     a
                ld      (dk_re),a
                jp      dkd_l
dk_pd:          ld      a,(dk_sel)
                add     a,10
                ld      b,a
                ld      a,(dk_n)
                dec     a
                cp      b
                jr      nc,dk_pd1
                ld      b,a
dk_pd1:         ld      a,b
                ld      (dk_sel),a
                jr      dk_scr
dk_pu:          ld      a,(dk_sel)
                sub     10
                jr      nc,dk_pu1
                xor     a
dk_pu1:         ld      (dk_sel),a
                jp      dk_scr
dk_ent:         ld      a,(dk_sel)
                call    dk_adr
                ld      a,(hl)
                cp      0FFh
                jr      z,dk_ok                 ; "<Extract here>" = 이 폴더에 풀기
                inc     hl
                ld      a,(hl)
                cp      '.'
                jr      z,dk_par                ; ".." → 위 폴더
                push    hl                      ; 폴더로 들어감
                ld      hl,dk_dir
                call    strlen
                ld      e,a
                ld      d,0
                ld      hl,dk_dir
                add     hl,de
                dec     hl
                ld      a,(hl)
                inc     hl
                cp      '\'
                jr      z,dk_e2
                ld      (hl),'\'
                inc     hl
dk_e2:          ex      de,hl
                pop     hl
                call    copy_z
                xor     a
                ld      (de),a
                jp      dk_scan
dk_ok:          ld      hl,dk_dir               ; 고른 폴더 → PROGBUF, 풀기(오버레이 13 진입 1)로
                ld      de,PROGBUF
                call    copy_z
                xor     a
                ld      (de),a
                call    clear_screen
                pop     hl
                ld      a,13
                ld      b,1
                jp      ov_go
dk_par:         ld      hl,dk_dir               ; 위 폴더: 마지막 '\' 에서 자름 (루트면 그대로)
                call    strlen
                cp      4
                jp      c,dk_key
                ld      e,a
                ld      d,0
                ld      hl,dk_dir
                add     hl,de
dk_p1:          dec     hl
                ld      a,(hl)
                cp      '\'
                jr      nz,dk_p1
                ld      de,dk_dir+3
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,dk_p2
                ld      hl,dk_dir+2             ; 루트 "A:\" 는 '\' 를 남김
                inc     hl
dk_p2:          ld      (hl),0
                jp      dk_scan
dk_root:        xor     a
                ld      (dk_dir+3),a
                jp      dk_scan
dk_ta:          and     0DFh                    ; 글자 → 그 글자로 시작하는 다음 폴더
                ld      (dk_ch),a
                ld      a,(dk_sel)
                inc     a
                ld      b,a
dk_t1:          ld      a,(dk_n)
                cp      b
                jr      z,dk_t2
                jr      c,dk_t2
                ld      a,b
                call    dk_adr
                inc     hl
                ld      a,(hl)
                and     0DFh
                ld      c,a
                ld      a,(dk_ch)
                cp      c
                jr      z,dk_t3
                inc     b
                jr      dk_t1
dk_t3:          ld      a,b
                ld      (dk_sel),a
                jp      dk_scr
dk_t2:          jp      dk_key
; ---- 새 폴더 만들기: 이름을 물어 8.3 (대문자·숫자·_ - 만, 8 글자까지)로 바꿔 dk_dir 안에 만들고, 목록을 다시 읽어 그 폴더에 커서
dk_mk:          xor     a
                ld      (inbuf),a
                ld      (in_len),a
                ld      (il_first),a
                ld      hl,dk_mkq
                call    input_line
                jp      nz,dk_draw              ; ESC: 상자 다시 그림
                call    trim_in
                ld      hl,inbuf                ; 8.3 이름 만들기 (점·빈칸은 건너뜀, 그 밖의 글자는 _)
                ld      de,dk_nm
                ld      b,8
dm_l:           ld      a,(hl)
                inc     hl
                or      a
                jr      z,dm_e
                cp      '.'
                jr      z,dm_l
                cp      ' '
                jr      z,dm_l
                cp      'a'
                jr      c,dm_u
                cp      'z'+1
                jr      nc,dm_u
                and     0DFh
dm_u:           cp      'A'
                jr      c,dm_d
                cp      'Z'+1
                jr      c,dm_ok
dm_d:           cp      '0'
                jr      c,dm_s
                cp      '9'+1
                jr      c,dm_ok
dm_s:           cp      '-'
                jr      z,dm_ok
                ld      a,'_'
dm_ok:          ld      (de),a
                inc     de
                djnz    dm_l
dm_e:           xor     a
                ld      (de),a
                ld      a,(dk_nm)
                or      a
                jp      z,dk_draw               ; 이름이 비었음
                ld      hl,dk_dir               ; 경로 = dk_dir + \ + 이름
                ld      de,specbuf
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,dm_p
                ld      a,'\'
                ld      (de),a
                inc     de
dm_p:           ld      hl,dk_nm
                call    copy_z
                xor     a
                ld      (de),a
                ld      de,specbuf
                xor     a
                ld      b,10h
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,dm_made
                ld      hl,dk_tbad              ; 만들 수 없음 (이미 있음·쓰기 금지·공간 없음 등)
                ld      de,txt_ppok
                call    popup_ask
                jp      dk_draw
dm_made:        call    dk_read                 ; 목록을 다시 읽고 새 폴더에 커서
                xor     a
                ld      (dk_sel),a
                ld      (dk_top),a
                ld      b,1
dm_f:           ld      a,(dk_n)
                cp      b
                jp      z,dk_draw
                jp      c,dk_draw
                push    bc
                ld      a,b
                call    dk_adr
                inc     hl
                ld      de,dk_nm
                call    str_eq
                pop     bc
                jr      z,dm_hit
                inc     b
                jr      dm_f
dm_hit:         ld      a,b
                ld      (dk_sel),a
                cp      10
                jp      c,dk_draw
                sub     9
                ld      (dk_top),a
                jp      dk_draw
dk_mkq:         db      "New folder name (8.3):",0
dk_tbad:        db      "Cannot make the folder",0
dk_nm:          ds      10
; A = 번호 → HL = 항목 자리 (DKBUF + 번호 x 14), BC·DE 보존
dk_adr:         push    bc
                ld      l,a
                ld      h,0
                add     hl,hl
                ld      b,h
                ld      c,l
                add     hl,hl
                add     hl,hl
                add     hl,hl
                or      a
                sbc     hl,bc
                ld      bc,DKBUF
                add     hl,bc
                pop     bc
                ret
; 폴더 dk_dir 의 하위 폴더를 DKBUF 에 모음 (0 = "<Extract here>", 1 = "..", 폴더들)
dk_read:        xor     a
                call    dk_adr
                ld      (hl),0FFh
                inc     hl
                ld      (hl),0
                ld      a,1
                ld      (dk_n),a
                ld      hl,dk_dir
                call    strlen
                cp      4
                jr      c,dr_p1
                ld      a,(dk_n)
                call    dk_adr
                ld      (hl),10h
                inc     hl
                ld      (hl),'.'
                inc     hl
                ld      (hl),'.'
                inc     hl
                ld      (hl),0
                ld      hl,dk_n
                inc     (hl)
dr_p1:          ld      hl,dk_dir
                ld      de,specbuf
                call    copy_z
                dec     de
                ld      a,(de)
                inc     de
                cp      '\'
                jr      z,dr_s
                ld      a,'\'
                ld      (de),a
                inc     de
dr_s:           ld      hl,txt_star
                ld      bc,4
                ldir
                ld      de,specbuf
                ld      b,SRCHATTR
                ld      c,F_FFIRST
                call    bdos_fib
                ret     nz
dr_l:           ld      a,(FIB+14)
                and     08h
                jr      nz,dr_n
                ld      a,(FIB+14)
                and     10h
                jr      z,dr_n                  ; 폴더만
                ld      a,(FIB+1)
                cp      '.'
                jr      z,dr_n
                ld      a,(dk_n)
                cp      DKMAX
                jr      nc,dr_n
                call    dk_adr
                ld      (hl),10h
                inc     hl
                ex      de,hl
                ld      hl,FIB+1
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,dk_n
                inc     (hl)
dr_n:           ld      c,F_FNEXT
                call    bdos_fib
                jr      z,dr_l
                ret
dk_ttl:         db      "Extract to: ",0
dk_hint:        db      "Enter open/OK  BS up  F7 new folder  Esc cancel",0
dk_okt:         db      "<Extract here>",0
dk_dir:         ds      64
dk_n:           db      0
dk_sel:         db      0
dk_top:         db      0
dk_i:           db      0
dk_r:           db      0
dk_re:          db      0
dk_old:         db      0
dk_ot:          db      0
dk_at:          db      0
dk_ch:          db      0
