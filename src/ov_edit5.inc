; 만든이: KimTaeWoo
; X86FC 편집기 큰 파일 명령 (오버레이 14) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 큰 파일(창 방식)의 창 옮기기·저장·첫 창 읽기·뒤쪽 창 찾기·줄 이동·전부 바꾸기.  오버레이 1(편집기) 또는 10(명령)이 건너옴.
;   명령(ed_cmd): 16 창 옮기기, 17 저장, 18 저장하고 닫기, 19 첫 창 읽기, 22 뒤쪽 창에서 찾기, 23 전부 바꾸기, 24 줄 이동 (bw_k = 줄 번호)
; 끝나면 편집기(1번 칸)로 돌아감.  쪽 2 에 글이 끼워진 동안 코드·자료는 8000h 아래여야 함 (3536 바이트 이하).
; ----------------------------------------------------------------------
                include "edvars.inc"
                include "edundo.inc"
                include "ed2lib.inc"

ed5_cmd:        ld      a,(ed_cmd)
                cp      16
                jp      z,c_slide
                cp      17
                jp      z,c_wsave
                cp      18
                jp      z,c_wexit
                cp      19
                jp      z,c_first
                cp      22
                jp      z,c_fnext
                cp      23
                jp      z,bw_rall
                cp      25
                jp      z,c_ssave
                cp      26
                jp      z,c_sexit
                ld      de,(bw_k)
                jp      gb_go
; ---- 작은 파일 저장 (명령 25 = 저장 후 계속, 26 = 저장하고 닫기)
c_ssave:        call    e2_map
                call    ed_wr
                call    e2_unmap
                jp      c_back
c_sexit:        call    e2_map
                call    ed_wr
                call    e2_unmap
                jp      c_wclose
; ---- 뒤쪽 창들에서 찾기 (앞 창에서 못 찾음)
c_fnext:        call    e2_map
cf_b:           ld      a,(bw_dn)
                or      a
                jr      z,c_nomore
                call    bw_next
                jr      nz,c_nomore
                ld      hl,EDBUF
                call    e2_scan
                jr      c,cf_b
                ld      (ed_cur),hl
                call    e2_settop
                call    e2_unmap
                jp      c_back
c_nomore:       call    e2_unmap
                ld      hl,txt_nf
                ld      a,(ed_rp)
                or      a
                jr      z,c_nm1
                ld      hl,txt_done
c_nm1:          xor     a
                ld      (ed_rp),a
                call    e2_msg
                jp      c_back
c_full:         xor     a
                ld      (ud_off),a
                call    e2_unmap
                ld      hl,txt_full
                call    e2_msg
                jp      c_back
txt_nf:         db      "Not found",0
txt_done:       db      "No more matches",0
txt_full:       db      "No room in the buffer",0
txt_repl:       db      " replaced",0
; ======================================================================
; (0.12.45) 큰 파일 (창 방식).  버퍼(EDBUF)에는 파일의 [win_off, win_off + win_dl) 토막만 있고(줄 단위로 자름), 고친 뒤 창을 옮기려면
; 먼저 파일 전체를 다시 쓴다 (앞부분 복사 + 버퍼 + 뒷부분 복사 → 임시 파일 X86FC.TMP → 원본에 되복사).
; ======================================================================
; ---- 32비트 보조: (HL) += DE / (HL) -= DE / HL 이 DE 보다 작으면 C
add32:          ld      a,(hl)
                add     a,e
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,d
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,0
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                adc     a,0
                ld      (hl),a
                ret
cmp32:          ld      bc,3
                add     hl,bc
                ex      de,hl
                add     hl,bc
                ex      de,hl
                ld      b,4
cm_l:           ld      a,(de)
                ld      c,a
                ld      a,(hl)
                cp      c
                ret     nz
                dec     hl
                dec     de
                djnz    cm_l
                or      a
                ret
; ---- 원본 열기·위치 이동·닫기 (핸들 ed_h)
bw_open:        ld      de,srcpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (ed_h),a
                xor     a
                ret
bw_seek:        ld      e,(hl)                  ; HL = 32비트 위치 자리
                inc     hl
                ld      d,(hl)
                inc     hl
                push    de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                pop     hl
                ld      a,(ed_h)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                jp      BDOS
bw_close:       ld      a,(ed_h)
                ld      b,a
                ld      c,F_CLOSE
                jp      BDOS
; ---- 창 읽기: bw_off 에서 LOADSZ 바이트 (bw_al = 1 이면 처음 부분 줄은 건너뜀), 뒤가 더 있으면 마지막 줄바꿈까지만 → NZ = 실패.  글 쪽이 끼워진 상태
bw_load:        call    bw_open
                ret     nz
                ld      hl,bw_off
                call    bw_seek
                ld      a,(ed_h)
                ld      b,a
                ld      de,EDBUF
                ld      hl,LOADSZ
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,bl_1
                or      a
                jr      z,bl_1
                ld      hl,0
bl_1:           ld      (bw_n),hl
                call    bw_close
                ld      a,(bw_al)
                or      a
                jr      z,bl_c
                ld      hl,bw_off
                ld      a,(hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                jr      z,bl_c
                ld      hl,EDBUF
                ld      bc,(bw_n)
                ld      a,b
                or      c
                jr      z,bl_c
                ld      a,0Ah
                cpir
                jr      nz,bl_c
                push    bc
                ld      hl,(bw_n)
                or      a
                sbc     hl,bc
                ex      de,hl                   ; DE = 건너뛴 바이트 수
                push    de
                ld      hl,bw_off
                call    add32
                pop     de
                ld      hl,EDBUF
                add     hl,de
                pop     bc
                ld      (bw_n),bc
                ld      a,b
                or      c
                jr      z,bl_c
                ld      de,EDBUF
                ldir
bl_c:           ld      hl,bw_off               ; 창 끝 위치 = bw_off + bw_n
                ld      de,bw_t
                ld      bc,4
                ldir
                ld      hl,bw_t
                ld      de,(bw_n)
                call    add32
                ld      hl,bw_t
                ld      de,fsz
                call    cmp32
                jr      nc,bl_eof
                ld      a,1
                ld      (bw_dn),a
                ld      hl,(bw_n)
                ld      a,h
                or      l
                jr      z,bl_e
                ld      de,EDBUF-1
                add     hl,de
                ld      bc,(bw_n)
                ld      a,0Ah
                cpdr
                jr      nz,bl_e
                inc     bc
                ld      (bw_n),bc
                jr      bl_e
bl_eof:         xor     a
                ld      (bw_dn),a
bl_e:           ld      hl,(bw_n)
                ld      (win_dl),hl
                ld      de,EDBUF
                add     hl,de
                ld      (ed_end),hl
                ld      hl,bw_off
                ld      de,win_off
                ld      bc,4
                ldir
                ld      hl,win_off
                ld      a,(hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                ld      a,0
                jr      z,bl_u
                inc     a
bl_u:           ld      (bw_up),a
                xor     a
                ret
; ---- 현재 창의 줄바꿈 수 DE (HL = 시작, BC = 길이)
bw_cnt:         ld      de,0
bc_l:           ld      a,b
                or      c
                ret     z
                ld      a,0Ah
                cpir
                ret     nz
                inc     de
                jp      pe,bc_l
                ret
; ---- 다음 창 읽기 (바로 뒤: win_off + win_dl): 고쳤으면 먼저 파일에 씀 → NZ = 실패.  win_ln 은 지나간 줄 수만큼 늘림.  글 쪽 끼워진 채로 돌아옴
bw_next:        ld      a,(ed_mod)
                or      a
                jr      z,bn_a
                call    bw_commit
                ret     nz
bn_a:           call    e2_map
                ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l
                ld      hl,EDBUF
                call    bw_cnt
                ld      hl,(win_ln)
                add     hl,de
                ld      (win_ln),hl
                ld      hl,win_off
                ld      de,bw_off
                ld      bc,4
                ldir
                ld      hl,bw_off
                ld      de,(win_dl)
                call    add32
                xor     a
                ld      (bw_al),a
                call    bw_load
                ret     nz
                ld      hl,EDBUF
                ld      (ed_cur),hl
                ld      (ed_top),hl
                ld      hl,0
                ld      (ln_p),hl
                ld      hl,LOG
                ld      (ud_top),hl
                xor     a
                ret
; ---- 임시 파일 경로 = 원본 폴더 + X86FC.TMP → CMDBUF
bw_tmpname:     ld      hl,srcpath
                ld      de,CMDBUF
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,CMDBUF
                ld      d,h
                ld      e,l
bt_s:           ld      a,(hl)
                or      a
                jr      z,bt_e
                inc     hl
                cp      '\'
                jr      z,bt_m
                cp      ':'
                jr      nz,bt_s
bt_m:           ld      d,h
                ld      e,l
                jr      bt_s
bt_e:           ld      hl,bw_tmpn
bt_c:           ld      a,(hl)
                ld      (de),a
                inc     hl
                inc     de
                or      a
                jr      nz,bt_c
                ret
bw_tmpn:        db      "X86FC.TMP",0
; ---- 복사: ed_h → bw_h2.  bw_cpn = bw_t 바이트, bw_cpe = 끝까지.  → NZ = 쓰기 실패
bw_cpn:         ld      hl,bw_t
                ld      a,(hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                inc     hl
                or      (hl)
                ret     z
                ld      hl,256
                ld      a,(bw_t+1)
                ld      b,a
                ld      a,(bw_t+2)
                or      b
                ld      b,a
                ld      a,(bw_t+3)
                or      b
                jr      nz,cn_r
                ld      a,(bw_t+1)
                or      a
                jr      nz,cn_r
                ld      a,(bw_t)
                ld      l,a
                ld      h,0
cn_r:           ld      (bw_n),hl
                ld      a,(ed_h)
                ld      b,a
                ld      de,BUFC
                ld      c,F_READ
                call    BDOS
                ld      (bw_k),hl
                ld      a,h
                or      l
                ret     z                       ; 끝 (모자란 파일): 그만
                ld      a,(bw_h2)
                ld      b,a
                ld      de,BUFC
                ld      hl,(bw_k)
                ld      c,F_WRITE
                call    BDOS
                or      a
                ret     nz
                ld      hl,bw_t
                ld      de,(bw_k)
                call    sub32
                jr      bw_cpn
bw_cpe:         ld      hl,0FFFFh               ; 끝까지 = 아주 큰 개수 (읽은 게 0 이면 그만)
                ld      (bw_t),hl
                ld      (bw_t+2),hl
                jp      bw_cpn
sub32:          ld      a,(hl)
                sub     e
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                sbc     a,d
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                sbc     a,0
                ld      (hl),a
                inc     hl
                ld      a,(hl)
                sbc     a,0
                ld      (hl),a
                ret
; ---- 임시 파일(CMDBUF)을 원본(srcpath)에 되복사하고 임시 파일을 지움 → NZ = 실패 (임시 파일은 남김)
bw_back:        ld      de,CMDBUF
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (ed_h),a
                ld      a,(ed_attr)
                ld      b,a
                xor     a
                ld      de,srcpath
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      nz,bk_x
                ld      a,b
                ld      (bw_h2),a
                call    bw_cpe
                push    af
                ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
bk_x:           push    af
                call    bw_close
                pop     af
                ret     nz
                ld      de,CMDBUF
                ld      c,F_DELETE
                call    BDOS
                xor     a
                ret
; ---- 고친 창을 파일에 반영: 임시 파일에 [앞부분][버퍼][뒷부분] → 원본에 되복사.  끝나면 ed_mod = 0, 디스크 크기·창 길이 갱신.  글 쪽은 풀린 채로 돌아옴 → NZ = 실패(알림 띄움)
bw_commit:      call    e2_map
                call    bw_tmpname
                ld      de,CMDBUF
                xor     a
                ld      b,a
                ld      c,F_CREATE
                call    BDOS
                or      a
                jp      nz,bc_fail
                ld      a,b
                ld      (bw_h2),a
                call    bw_open
                jr      nz,bc_f2
                ld      hl,win_off              ; 앞부분
                ld      de,bw_t
                ld      bc,4
                ldir
                call    bw_cpn
                jr      nz,bc_f3
                ld      hl,(ed_end)             ; 버퍼
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (bw_n),hl
                ld      a,h
                or      l
                jr      z,bc_t
                ld      a,(bw_h2)
                ld      b,a
                ld      de,EDBUF
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      nz,bc_f3
bc_t:           ld      hl,win_off              ; 뒷부분: 원래 창 끝부터 끝까지
                ld      de,bw_t
                ld      bc,4
                ldir
                ld      hl,bw_t
                ld      de,(win_dl)
                call    add32
                ld      hl,bw_t
                call    bw_seek
                call    bw_cpe
                jr      nz,bc_f3
                call    bw_close
                ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                call    bw_back
                jr      nz,bc_fail
                ld      hl,(ed_end)             ; 창 길이 = 버퍼 길이, 디스크 크기는 다시 읽음
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (win_dl),hl
                call    bw_fszfix
                xor     a
                ld      (ed_mod),a
                call    e2_unmap
                xor     a
                ret
bc_f3:          push    af
                call    bw_close
                pop     af
bc_f2:          push    af
                ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
bc_fail:        call    e2_unmap
                ld      hl,txt_svf
                call    e2_msg
                or      1
                ret
; 원본 크기를 다시 읽어 fsz 에 (끝으로 이동 = 크기)
bw_fszfix:      ld      de,srcpath              ; 원본을 열어 끝으로 가서 크기를 읽음
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                ret     nz
                ld      a,b
                ld      (ed_h),a
                ld      de,0
                ld      hl,0
                ld      a,(ed_h)
                ld      b,a
                ld      a,2                     ; 끝 기준 0 = 크기
                ld      c,F_SEEK
                call    BDOS
                ld      (fsz),hl
                ld      (fsz+2),de
                jp      bw_close
; ---- 명령 17/18: 큰 파일 저장 (계속 / 저장하고 닫기)
c_wsave:        call    bw_commit
                jp      c_back
c_wexit:        call    bw_commit
                jp      nz,c_back
c_wclose:       call    hk_off
                xor     a
                ld      (hk_st),a
                ld      a,(ed_seg)
                ld      b,0
                ld      hl,(mp_fre)
                call    jphl3
                ld      a,(clip_seg)
                or      a
                jr      z,wc_n
                ld      b,0
                ld      hl,(mp_fre)
                call    jphl3
wc_n:
                xor     a
                ld      (CSRSW),a
                ret
jphl3:          jp      (hl)
; ---- 명령 19: 첫 창 읽기 (do_edit 가 큰 파일이면 여기로)
c_first:        call    e2_map
                ld      hl,0
                ld      (bw_off),hl
                ld      (bw_off+2),hl
                xor     a
                ld      (bw_al),a
                call    bw_load
                jr      z,cf_ok
                call    e2_unmap
                jr      c_wclose
cf_ok:          ld      hl,EDBUF
                ld      (ed_cur),hl
                ld      (ed_top),hl
                call    e2_unmap
                jp      c_back
; ---- 명령 16: 창 옮기기 (커서가 창 끝·앞에 다가옴).  커서의 파일 위치는 그대로, 새 창에서 커서가 KEEP 바이트 자리에 오게 함
c_slide:        ld      a,(ed_mod)
                or      a
                jr      z,sl_a
                call    bw_commit
                jp      nz,c_back
sl_a:           call    e2_map
                ld      hl,(ed_cur)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ex      de,hl                   ; DE = 커서의 창 안 위치
                push    de
                ld      hl,(ed_cur)
                ld      de,(ed_top)
                or      a
                sbc     hl,de
                ld      (bw_d),hl               ; 커서 - 화면 맨 위
                ld      hl,win_off
                ld      de,bw_c
                ld      bc,4
                ldir
                pop     de
                ld      hl,bw_c
                call    add32                   ; bw_c = 커서의 파일 위치
                ld      hl,win_off
                ld      de,bw_p
                ld      bc,4
                ldir                            ; bw_p = 옛 창 시작
                ld      hl,bw_c                 ; 새 창 시작 후보 = 커서 - KEEP (0 아래로는 안 감)
                ld      de,bw_off
                ld      bc,4
                ldir
                ld      hl,bw_c
                ld      de,kp32
                call    cmp32
                jr      nc,sl_s
                ld      hl,0                    ; 커서가 KEEP 앞이면 파일 처음
                ld      (bw_off),hl
                ld      (bw_off+2),hl
                jr      sl_c
sl_s:           ld      hl,bw_off
                ld      de,KEEP
                call    sub32
sl_c:           ld      hl,bw_off               ; 후보 < 옛 창 시작 이면 위로, 아니면 아래로
                ld      de,win_off
                call    cmp32
                jr      c,sl_up
                ld      hl,(bw_off)             ; 아래로: 메모리 안에서 줄 처음에 맞춤
                ld      de,(win_off)
                or      a
                sbc     hl,de
                ld      de,EDBUF
                add     hl,de
sl_f:           ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,sl_fe
                ld      a,(hl)
                inc     hl
                cp      0Ah
                jr      nz,sl_f
sl_fe:          push    hl
                or      a
                ld      de,EDBUF
                sbc     hl,de
                ld      (bw_k),hl               ; 건너뛴 바이트
                ld      b,h
                ld      c,l
                ld      hl,EDBUF
                call    bw_cnt
                ld      hl,(win_ln)
                add     hl,de
                ld      (win_ln),hl
                ld      hl,win_off
                ld      de,bw_off
                ld      bc,4
                ldir
                ld      hl,bw_off
                ld      de,(bw_k)
                call    add32
                pop     hl
                xor     a
                ld      (bw_al),a
                jr      sl_ld
sl_up:          ld      a,1
                ld      (bw_al),a
sl_ld:          call    bw_load
                jp      nz,c_back
                ld      a,(bw_al)
                or      a
                jr      z,sl_set
                ld      hl,(bw_p)               ; 위로: 새 시작 ~ 옛 시작 사이의 줄바꿈 수만큼 win_ln 감소
                ld      de,(win_off)
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l
                ld      hl,(win_dl)
                or      a
                sbc     hl,bc
                jr      nc,sl_u1
                ld      bc,(win_dl)
sl_u1:          ld      hl,EDBUF
                call    bw_cnt
                ld      hl,(win_ln)
                or      a
                sbc     hl,de
                ld      (win_ln),hl
sl_set:         ld      hl,bw_c                 ; 커서 = EDBUF + (커서 파일 위치 - 새 창 시작)
                ld      de,bw_t
                ld      bc,4
                ldir
                ld      hl,(bw_t)
                ld      de,(win_off)
                or      a
                sbc     hl,de
                ld      de,EDBUF
                add     hl,de
                ld      de,(ed_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,sl_k
                ld      hl,(ed_end)
sl_k:           ld      (ed_cur),hl
                ld      de,(bw_d)
                or      a
                sbc     hl,de
                jr      c,sl_t
                ld      de,EDBUF
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      c,sl_t
                ld      (ed_top),hl
                jr      sl_e
sl_t:           call    e2_settop
sl_e:           ld      hl,0
                ld      (ln_p),hl
                ld      (sel_a),hl
                ld      hl,LOG
                ld      (ud_top),hl
                call    e2_unmap
                jp      c_back
kp32:           dw      KEEP,0
; ---- 줄 이동 (큰 파일): DE = 줄 번호.  창 안이면 거기서, 앞이면 처음 창부터, 뒤면 창을 차례로 읽으며 셈
gb_go:          ld      (bw_k),de
                ld      hl,(win_ln)
                ex      de,hl
                ld      hl,(bw_k)
                or      a
                sbc     hl,de                   ; HL = N - win_ln
                jr      nc,gb_a
                ld      a,(ed_mod)              ; 앞쪽 줄: 처음 창부터
                or      a
                jr      z,gb_f
                call    bw_commit
                jp      nz,c_back
gb_f:           call    e2_map
                ld      hl,0
                ld      (bw_off),hl
                ld      (bw_off+2),hl
                xor     a
                ld      (bw_al),a
                call    bw_load
                jp      nz,c_back
                ld      hl,1
                ld      (win_ln),hl
                ld      hl,(bw_k)
                dec     hl
gb_a:           ld      d,h
                ld      e,l                     ; DE = 건너뛸 줄바꿈 수
                call    e2_map
                ld      hl,EDBUF
gb_l:           ld      a,d
                or      e
                jr      z,gb_h
                ld      bc,(ed_end)
                ld      a,c
                sub     l
                ld      c,a
                ld      a,b
                sbc     a,h
                ld      b,a
                or      c
                jr      z,gb_x
                ld      a,0Ah
                cpir
                jr      nz,gb_x
                dec     de
                jr      gb_l
gb_x:           ld      a,(bw_dn)
                or      a
                jr      z,gb_h
                push    de
                call    bw_next
                pop     de
                jp      nz,c_back
                ld      hl,EDBUF
                jr      gb_l
gb_h:           ld      (ed_cur),hl
                ld      (ed_top),hl
                ld      hl,0
                ld      (ln_p),hl
                call    e2_unmap
                jp      c_back
; ---- 전부 바꾸기 (큰 파일): 처음부터 창마다 바꿔 임시 파일에 쓰고 되복사 (되돌릴 수 없음)
bw_rall:        ld      a,(ed_mod)
                or      a
                jr      z,br_a
                call    bw_commit
                jp      nz,c_back
br_a:           call    bw_tmpname
                ld      de,CMDBUF
                xor     a
                ld      b,a
                ld      c,F_CREATE
                call    BDOS
                or      a
                jp      nz,bc_fail
                ld      a,b
                ld      (bw_h2),a
                ld      hl,0
                ld      (ed_cnt),hl
                ld      (bw_off),hl
                ld      (bw_off+2),hl
                ld      (bw_c),hl
                ld      (bw_c+2),hl
                xor     a
                ld      (bw_al),a
                inc     a
                ld      (ud_off),a
br_w:           call    e2_map
                call    bw_load
                jp      nz,br_f
                ld      hl,EDBUF
br_r:           call    e2_scan
                jr      c,br_d
                ld      (ed_cur),hl
                call    e2_rep
                jp      c,br_f
                ld      hl,(ed_cnt)
                inc     hl
                ld      (ed_cnt),hl
                ld      hl,(ed_cur)
                ld      a,(rs_n)
                ld      e,a
                ld      d,0
                add     hl,de
                jr      br_r
br_d:           ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (bw_n),hl
                push    hl
                ld      d,h
                ld      e,l
                ld      hl,bw_c
                call    add32
                pop     hl
                ld      a,h
                or      l
                jr      z,br_n
                ld      a,(bw_h2)
                ld      b,a
                ld      de,EDBUF
                ld      c,F_WRITE
                call    BDOS
                or      a
                jp      nz,br_f
br_n:           ld      a,(bw_dn)
                or      a
                jr      z,br_e
                ld      hl,win_off
                ld      de,bw_off
                ld      bc,4
                ldir
                ld      hl,bw_off
                ld      de,(win_dl)
                call    add32
                jr      br_w
br_e:           ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                call    bw_back
                jr      nz,br_x
                xor     a
                ld      (ud_off),a
                call    bw_fszfix
                ld      hl,0                    ; 첫 창 다시 읽기
                ld      (bw_off),hl
                ld      (bw_off+2),hl
                call    e2_map
                call    bw_load
                ld      hl,EDBUF
                ld      (ed_cur),hl
                ld      (ed_top),hl
                ld      hl,1
                ld      (win_ln),hl
                ld      hl,0
                ld      (ln_p),hl
                ld      hl,LOG
                ld      (ud_top),hl
                xor     a
                ld      (ed_mod),a
                ld      (ed_rp),a
                call    e2_unmap
                ld      de,errbuf
                ld      hl,(ed_cnt)
                call    put_dec16
                ld      hl,txt_repl
                call    copy_z
                xor     a
                ld      (de),a
                ld      hl,errbuf
                call    e2_msg
                jp      c_back
br_f:           ld      a,(bw_h2)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
br_x:           xor     a
                ld      (ud_off),a
                jp      bc_fail
txt_svf:        db      "Save failed (see X86FC.TMP)",0
; 파일 쓰기: 열어서(읽기·쓰기) 처음부터 쓰고, 짧아졌으면 그 자리에서 잘라 냄
ed_wr:          ld      de,srcpath              ; 같은 이름으로 다시 만듦(내용 비움, 속성 유지)
                ld      a,(ed_attr)
                ld      b,a
                xor     a
                ld      c,F_CREATE
                call    BDOS
                or      a
                jr      z,ew_o
                call    explain_err
                or      1
                ret
ew_o:           ld      a,b
                ld      (ed_h),a
                ld      hl,(ed_end)
                ld      de,EDBUF
                or      a
                sbc     hl,de
                ld      (ed_wn),hl
                ld      a,h
                or      l
                jr      z,ew_t
                ld      de,EDBUF
                ld      a,(ed_h)
                ld      b,a
                ld      c,F_WRITE
                call    BDOS
                or      a
                jr      nz,ew_e
ew_t:           xor     a
ew_e:           push    af
                ld      a,(ed_h)
                ld      b,a
                ld      c,F_CLOSE
                call    BDOS
                pop     af
                or      a
                jr      z,ew_ok
                call    explain_err
                or      1
                ret
ew_ok:          xor     a
                ld      (ed_mod),a
                ld      hl,txt_edsaved
                ld      (msg_p),hl
                xor     a
                ret
txt_edsaved:    db      "Saved",0
