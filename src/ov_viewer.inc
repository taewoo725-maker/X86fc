; 만든이: KimTaeWoo
; X86FC F2 텍스트 보기 (오버레이 4) - SPDX-License-Identifier: GPL-3.0-or-later
; ----------------------------------------------------------------------
; 파일을 한 덩어리(8점 줄 보기 2000 바이트, 7H 3400 바이트)씩 VBUF 에 읽어 가며 보여 줌.
;  위/아래 = 한 줄, 왼쪽/오른쪽/SPACE = 한 쪽, HOME = 처음, B = 끝, ESC·F10·Q = 닫기
;  8점 줄 (SCREEN 0 / SCREEN 7): 제목 + 22 줄 + 안내, 80 칸, 한 바이트 = 한 칸 (80h 이상 = '.')
;  7H (SCREEN 7 한글, 줄 높이 10점, s7_h 켠 채로 그림): 제목 + 19 줄 + 안내 = 21 줄.
;    글 인코딩을 첫 덩어리로 가려냄: UTF-8 (BOM 건너뜀) / EUC-KR(KS X 1001 한글 완성형) / 그 밖 = ASCII.
;    한글 음절 = 글자 칸 2개 (한글 쌍 80h+(idx>>7), 80h+(idx AND 7Fh)), 그 밖 UTF-8·KS X 1001 글자 = '?' 한 칸.
;    EUC-KR 은 X86FC.HGL 뒤쪽 표(ck 37 x 2 + 차이 2350 바이트)를 CMDBUF 에 읽어 idx 를 구함.
; ----------------------------------------------------------------------
V_TAB           equ     CMDBUF                  ; EUC-KR 표 자리 (보기 동안 CMDBUF~CPBUF 끝을 씀)
V_CKN           equ     37
V_TABN          equ     V_CKN*2+2350

do_view:        call    cur_ix
                ld      a,(ix+PS_COUNT)
                or      (ix+PS_COUNT+1)
                ret     z
                ld      l,(ix+PS_CUR)
                ld      h,(ix+PS_CUR+1)
                call    ptr_get
                ld      (en_p),hl
                ld      de,EN_ATTR
                add     hl,de
                bit     4,(hl)
                jr      z,dv_f
                ld      hl,txt_notfile
                ld      (msg_p),hl
                ret
dv_f:           ld      hl,(en_p)
                ld      de,EN_SIZE
                add     hl,de
                ld      de,v_size
                ld      bc,4
                ldir
                ld      hl,(en_p)
                call    disp_name
                ld      hl,nmbuf
                ld      de,v_name
                ld      bc,16
                ldir
                call    cur_ix
                push    ix
                pop     hl
                ld      de,PS_PATH
                add     hl,de
                ld      de,srcpath
                call    mkpath
                ld      de,srcpath
                ld      a,1
                ld      c,F_OPEN
                call    BDOS
                or      a
                jr      z,dv_o
                call    explain_err
                ret
dv_o:           ld      a,b
                ld      (v_h),a
                ld      hl,0
                ld      (v_top),hl
                ld      (v_top+2),hl
                ld      (v_want),hl
                ld      (v_want+2),hl
                ld      (v_clen),hl
                call    v_mode
                call    clear_screen
                call    v_foot_draw
dv_draw:        call    v_render
dv_key:         call    wait_key
                cp      1Bh
                jr      z,dv_end
                cp      K_F10
                jr      z,dv_end
                cp      'q'
                jr      z,dv_end
                cp      'Q'
                jr      z,dv_end
                cp      K_DOWN
                jr      z,dv_dn
                cp      K_UP
                jr      z,dv_up
                cp      K_RIGHT
                jr      z,dv_pd
                cp      ' '
                jr      z,dv_pd
                cp      K_LEFT
                jr      z,dv_pu
                cp      K_HOME
                jr      z,dv_top
                cp      'b'
                jr      z,dv_bot
                cp      'B'
                jr      z,dv_bot
                cp      06h                     ; Ctrl+F = 찾기, n / F3 = 다음, p = 이전
                jp      z,dv_find
                cp      'n'
                jp      z,dv_nxt
                cp      'N'
                jp      z,dv_nxt
                cp      10h
                jp      z,dv_nxt
                cp      'p'
                jp      z,dv_prv
                cp      'P'
                jp      z,dv_prv
                jr      dv_key
dv_end:         ld      a,(v_h)
                ld      b,a
                ld      c,F_CLOSE
                jp      BDOS
dv_dn:          ld      a,(v_more)
                or      a
                jr      z,dv_key
                ld      hl,v_nxt
                call    v_set_top
                jr      dv_draw
dv_pd:          ld      a,(v_more)
                or      a
                jr      z,dv_key
                ld      hl,v_aft
                call    v_set_top
                jr      dv_draw
dv_up:          call    v_up1
                jr      dv_draw
dv_pu:          ld      a,(v_body)
                ld      b,a
dv_pul:         push    bc
                call    v_up1
                pop     bc
                djnz    dv_pul
                jp      dv_draw
dv_top:         ld      hl,0
                ld      (v_top),hl
                ld      (v_top+2),hl
                jp      dv_draw
dv_bot:         ld      a,(s7_h)
                or      a
                jr      z,dv_bot8
                ld      hl,(v_size)             ; 7H: 끝에서 한 쪽(본문 줄 수) 만큼 줄을 거슬러 올라감 (글자 폭이 달라 바이트 수로는 못 맞춤)
                ld      (v_top),hl
                ld      hl,(v_size+2)
                ld      (v_top+2),hl
                ld      a,(v_body)
                ld      b,a
db_hl:          push    bc
                call    v_up1
                pop     bc
                djnz    db_hl
                jp      dv_draw
dv_bot8:        ld      hl,(v_size)             ; 끝에서 한 쪽쯤 앞에서 줄 시작을 찾아 맞춤
                ld      de,(v_size+2)
                ld      bc,(v_bback)
                or      a
                sbc     hl,bc
                jr      nc,db_s
                ld      a,d
                or      e
                jr      z,db_z
                dec     de
db_s:           ld      (v_top),hl
                ld      (v_top+2),de
                ld      hl,v_top
                ld      de,v_want
                ld      bc,4
                ldir
                call    v_load0
                ld      hl,VBUF
                ld      de,(v_clen)
                ld      b,d
                ld      c,e
                ld      a,b
                or      c
                jp      z,dv_draw
                ld      a,0Ah
                cpir                            ; 첫 줄바꿈 뒤부터
                jp      nz,dv_draw
                ld      de,VBUF
                or      a
                sbc     hl,de                   ; HL = 줄바꿈 다음 위치(번호)
                ld      b,h
                ld      c,l
                ld      hl,(v_top)
                add     hl,bc
                ld      (v_top),hl
                jp      nc,dv_draw
                ld      hl,(v_top+2)
                inc     hl
                ld      (v_top+2),hl
                jp      dv_draw
db_z:           ld      hl,0
                ld      (v_top),hl
                ld      (v_top+2),hl
                jp      dv_draw
v_set_top:      ld      de,v_top
                ld      bc,4
                ldir
                ret

; ---- 방식 정하기: 8점 줄이면 22 줄, 7H 이면 19 줄 + 글 인코딩 가려내기
v_mode:         xor     a
                ld      (v_enc),a
                ld      (v_inc),a
                ld      hl,2000
                ld      (v_chunk),hl
                ld      hl,1760
                ld      (v_bback),hl
                ld      a,22
                ld      (v_body),a
                ld      a,23
                ld      (v_foot),a
                ld      a,(s7_h)
                or      a
                ret     z
                ld      hl,3400
                ld      (v_chunk),hl
                ld      hl,2400
                ld      (v_bback),hl
                ld      a,19
                ld      (v_body),a
                ld      a,20
                ld      (v_foot),a
                call    v_load0
                call    v_sc8
                jr      c,vm_e
                ld      a,1
                ld      (v_enc),a
                ret
vm_e:           call    v_sce
                ret     z
; EUC-KR 표를 X86FC.HGL 의 201096 바이트 뒤에서 V_TAB 로 읽음 (없으면 ASCII 로 둠)
v_tabload:      ld      a,(hgl_h)
                cp      0FFh
                ret     z
                ld      b,a
                ld      de,3                    ; (0.12.14) OVL 안 글꼴 뒤의 EUC-KR 표
                ld      hl,HGB+201096-196608    ; (= OVL 안 EUC-KR 표 위치 - 3 x 65536)
                xor     a
                ld      c,F_SEEK
                call    BDOS
                or      a
                ret     nz
                ld      a,(hgl_h)
                ld      b,a
                ld      de,V_TAB
                ld      hl,V_TABN
                ld      c,F_READ
                call    BDOS
                or      a
                ret     nz
                ld      de,V_TABN
                or      a
                sbc     hl,de
                ret     nz
                ld      a,2
                ld      (v_enc),a
                ret
; UTF-8 으로 올바른가 (덩어리 끝에서 잘린 것은 봐줌) → NC = 맞음
v_sc8:          ld      hl,VBUF
                ld      bc,(v_clen)
s8_l:           ld      a,b
                or      c
                ret     z
                ld      a,(hl)
                inc     hl
                dec     bc
                cp      80h
                jr      c,s8_l
                cp      0C2h
                jr      c,s8_bad
                ld      d,1
                cp      0E0h
                jr      c,s8_n
                inc     d
                cp      0F0h
                jr      c,s8_n
                inc     d
                cp      0F5h
                jr      nc,s8_bad
s8_n:           ld      a,b
                or      c
                ret     z
                ld      a,(hl)
                and     0C0h
                cp      80h
                jr      nz,s8_bad
                inc     hl
                dec     bc
                dec     d
                jr      nz,s8_n
                jr      s8_l
s8_bad:         scf
                ret
; EUC-KR 인가 (높은 바이트가 모두 A1~FE 두 개 짝이고 한글 짝이 하나 이상) → NZ = 맞음
v_sce:          ld      hl,VBUF
                ld      bc,(v_clen)
                ld      e,0
se_l:           ld      a,b
                or      c
                jr      z,se_d
                ld      a,(hl)
                inc     hl
                dec     bc
                cp      80h
                jr      c,se_l
                cp      0A1h
                jr      c,se_bad
                cp      0FFh
                jr      nc,se_bad
                ld      d,a
                ld      a,b
                or      c
                jr      z,se_d
                ld      a,(hl)
                inc     hl
                dec     bc
                cp      0A1h
                jr      c,se_bad
                cp      0FFh
                jr      nc,se_bad
                ld      a,d
                cp      0B0h
                jr      c,se_l
                cp      0C9h
                jr      nc,se_l
                inc     e
                jr      se_l
se_d:           ld      a,e
                or      a
                ret
se_bad:         xor     a
                ret

; ---- 안내 줄
v_foot_draw:    ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      hl,txt_vfoot
                ld      de,hbuf+2
                call    copy_z
                ld      a,(v_foot)
                call    row_addr
                ld      de,hbuf
                ld      b,80
                jp      scr_write

; v_want 위치에서 v_chunk 바이트를 VBUF 에 읽음 (이미 읽어 둔 곳이면 건너뜀). v_eof = 파일 끝까지 읽었음
v_load0:        ld      hl,(v_clen)
                ld      a,h
                or      l
                jr      z,vl_do
                ld      hl,(v_want)
                ld      de,(v_coff)
                or      a
                sbc     hl,de
                jr      nz,vl_do
                ld      hl,(v_want+2)
                ld      de,(v_coff+2)
                or      a
                sbc     hl,de
                ret     z
vl_do:          ld      de,(v_want+2)
                ld      hl,(v_want)
                ld      a,(v_h)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(v_h)
                ld      b,a
                ld      de,VBUF
                ld      hl,(v_chunk)
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,vl_ok
                or      a
                jr      z,vl_ok
                ld      hl,0
vl_ok:          ld      (v_clen),hl
                ld      de,(v_chunk)
                or      a
                sbc     hl,de
                ld      a,0
                adc     a,0                     ; 덜 읽었으면 1 (= 파일 끝)
                ld      (v_eof),a
                ld      hl,(v_want)
                ld      (v_coff),hl
                ld      hl,(v_want+2)
                ld      (v_coff+2),hl
                ret

; ---- 화면 그리기
v_render:       ld      hl,v_top
                ld      de,v_want
                ld      bc,4
                ldir
                call    v_load0
                ld      a,(v_eof)
                ld      (v_inc),a
                ld      hl,VBUF
                ld      (v_ptr),hl
                ld      de,(v_clen)
                add     hl,de
                ld      (v_end),hl
                call    v_head
                xor     a
                ld      (v_row),a
vr_l:           call    v_row_build
                ld      a,(v_row)
                inc     a
                call    row_addr
                ld      de,hbuf
                ld      b,80
                call    scr_write
                ld      a,(v_row)
                or      a
                jr      nz,vr_n
                ld      hl,(v_ptr)              ; 한 줄 내린 위치 기억
                ld      de,-VBUF
                add     hl,de
                ld      de,v_nxt
                call    v_mark
vr_n:           ld      a,(v_row)
                inc     a
                ld      (v_row),a
                ld      hl,v_body
                cp      (hl)
                jr      c,vr_l
                ld      hl,(v_ptr)              ; 한 쪽 내린 위치 기억
                ld      de,-VBUF
                add     hl,de
                ld      de,v_aft
                call    v_mark
                ld      hl,(v_ptr)
                ld      de,(v_end)
                or      a
                sbc     hl,de
                jr      c,vr_more
                ld      a,(v_eof)               ; 읽은 것을 다 썼음: 가득 읽었으면 뒤에 더 있을 수 있음
                or      a
                jr      nz,vr_no
vr_more:        ld      a,1
                ld      (v_more),a
                ret
vr_no:          xor     a
                ld      (v_more),a
                ret
; HL = 더할 값, DE = 쓸 자리(4 바이트) → v_top + HL
v_mark:         ld      (v_dlt),hl
                ld      (v_dst),de
                ld      hl,(v_top)
                ld      bc,(v_dlt)
                add     hl,bc
                ld      ix,(v_dst)
                ld      (ix+0),l
                ld      (ix+1),h
                ld      hl,(v_top+2)
                jr      nc,vm_n
                inc     hl
vm_n:           ld      (ix+2),l
                ld      (ix+3),h
                ret

; ---- 화면 한 줄(hbuf, 80 칸) 만들기: v_ptr 에서 v_end 까지 읽어 감 (끝나면 v_ptr = 다음 줄 처음)
v_row_build:    ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      c,0
                ld      hl,(v_ptr)
                ld      de,(v_end)
vb_c:           push    hl
                or      a
                sbc     hl,de
                pop     hl
                jp      nc,vb_done
                ld      a,(hl)
                inc     hl
                cp      0Ah
                jp      z,vb_done
                cp      0Dh
                jr      z,vb_c
                cp      1Ah
                jr      z,vb_c
                cp      09h
                jr      nz,vb_p
                ld      a,c
                or      7
                inc     a
                ld      c,a
                cp      80
                jr      c,vb_c
                jr      vb_full
vb_p:           cp      80h
                jr      nc,vb_hi
                cp      20h
                jr      c,vb_dot
                cp      7Fh
                jr      c,vb_ok
vb_dot:         ld      a,'.'
vb_ok:          call    vb_put
                ld      a,c
                cp      80
                jp      c,vb_c
vb_full:        push    hl                      ; 꽉 찬 줄 바로 뒤의 줄바꿈은 먹음
                or      a
                sbc     hl,de
                pop     hl
                jp      nc,vb_done
                ld      a,(hl)
                cp      0Dh
                jr      nz,vb_f2
                inc     hl
vb_f2:          push    hl
                or      a
                sbc     hl,de
                pop     hl
                jp      nc,vb_done
                ld      a,(hl)
                cp      0Ah
                jp      nz,vb_done
                inc     hl
vb_done:        ld      (v_ptr),hl
                ret
; A = 칸 글자 → hbuf[C], C+1  (HL·DE 보존)
vb_put:         push    hl
                ld      hl,hbuf
                ld      b,0
                add     hl,bc
                ld      (hl),a
                pop     hl
                inc     c
                ret
; ---- 80h 이상 바이트 (HL = 그 다음, vb_ld = 첫 바이트): 인코딩 별로
vb_hi:          ld      (vb_ld),a
                dec     hl
                ld      (vb_cs),hl
                inc     hl
                ld      a,(v_enc)
                or      a
                jr      z,vb_dot
                dec     a
                jp      nz,vb_euc
                ld      a,(vb_ld)               ; --- UTF-8
                cp      0C2h
                jr      c,vb_dot
                cp      0F5h
                jr      nc,vb_dot
                cp      0E0h
                jr      nc,vb_u34
                ld      a,1                     ; C2~DF = 두 바이트 → ?
                call    vb_ucont
                or      a
                jr      z,vb_q
vb_bad:         dec     a
                jr      z,vb_dot                ; 1 = 잘못된 바이트 → '.'
                ld      hl,(vb_cs)              ; 2 = 덩어리 끝에서 잘림 → 이 글자는 다음 쪽 처음
                jp      vb_done
vb_q:           ld      a,'?'
                jr      vb_ok
vb_u34:         cp      0F0h
                jr      c,vb_u3
                ld      a,3                     ; F0~F4 = 네 바이트 → ?
                call    vb_ucont
                or      a
                jr      z,vb_q
                jr      vb_bad
vb_u3:          ld      a,2
                call    vb_ucont
                or      a
                jr      nz,vb_bad
                ld      ix,(vb_cs)
                ld      a,(ix+0)
                cp      0EFh
                jr      nz,vb_u3h
                ld      a,(ix+1)
                cp      0BBh
                jr      nz,vb_u3h
                ld      a,(ix+2)
                cp      0BFh
                jp      z,vb_c                  ; BOM (U+FEFF) 은 건너뜀
vb_u3h:         ld      (vb_sp),hl
                ld      a,(ix+0)
                rlca
                rlca
                rlca
                rlca
                and     0F0h
                ld      h,a
                ld      a,(ix+1)
                and     3Fh
                ld      l,a
                rrca
                rrca
                and     0Fh
                or      h
                ld      h,a
                ld      a,l
                and     3
                rrca
                rrca
                ld      l,a
                ld      a,(ix+2)
                and     3Fh
                or      l
                ld      l,a                     ; HL = 유니코드
                ld      a,h
                sub     0ACh
                jr      c,vb_nh
                ld      h,a
                cp      2Bh
                jr      c,vb_pair
                jr      nz,vb_nh
                ld      a,l
                cp      0A4h
                jr      c,vb_pair
vb_nh:          ld      hl,(vb_sp)              ; 한글 음절이 아닌 글자 → ?
                jr      vb_q
; HL = idx (0~11171) 를 두 칸 쌍으로 (vb_sp = 다음 읽을 곳)
vb_pair:        ld      a,c
                cp      79
                jr      c,vb_pr
                ld      hl,(vb_cs)              ; 두 칸이 안 들어감 → 줄 끝, 이 글자는 다음 줄
                jp      vb_done
vb_pr:          ld      a,l
                rlca
                and     1
                ld      b,a
                ld      a,h
                add     a,a
                or      b
                add     a,80h
                call    vb_put
                ld      a,l
                or      80h
                call    vb_put
                ld      hl,(vb_sp)
                ld      a,c
                cp      80
                jp      c,vb_c
                jp      vb_full
vb_euc:         ld      a,(vb_ld)               ; --- EUC-KR
                cp      0A1h
                jp      c,vb_dot
                cp      0FFh
                jp      nc,vb_dot
                ld      a,1
                call    vb_avail
                jr      nc,vb_e2
                ld      a,(v_inc)
                or      a
                jp      nz,vb_dot
                ld      hl,(vb_cs)
                jp      vb_done
vb_e2:          ld      a,(hl)
                cp      0A1h
                jp      c,vb_dot
                cp      0FFh
                jp      nc,vb_dot
                ld      b,a
                inc     hl
                ld      (vb_sp),hl
                ld      a,(vb_ld)
                cp      0B0h
                jr      c,vb_nh
                cp      0C9h
                jr      nc,vb_nh
                sub     0B0h
                call    vb_eidx
                jr      vb_pair
; A = 첫 바이트 - B0h (0~24), B = 둘째 바이트 → HL = idx  (C 보존)
vb_eidx:        push    de
                ld      l,a
                ld      h,0
                ld      d,h
                ld      e,l
                add     hl,hl                   ; 2r
                push    hl
                add     hl,de                   ; 3r
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl                   ; 96r
                pop     de
                or      a
                sbc     hl,de                   ; 94r
                ld      a,b
                sub     0A1h
                ld      e,a
                ld      d,0
                add     hl,de                   ; n = KS X 1001 순번
                ld      (vb_n),hl
                ld      a,l
                and     3Fh
                ld      b,a                     ; 더할 차이 수
                ld      a,l
                and     0C0h
                ld      l,a
                ld      de,V_TAB+V_CKN*2+1
                add     hl,de                   ; 블록 첫 차이 다음 칸
                push    hl
                ld      hl,(vb_n)
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                srl     h
                rr      l
                res     0,l                     ; (n >> 6) x 2
                ld      de,V_TAB
                add     hl,de
                ld      e,(hl)
                inc     hl
                ld      d,(hl)                  ; DE = 블록 처음 idx
                pop     hl
                ld      a,b
                or      a
                jr      z,ve_d
ve_l:           ld      a,(hl)
                add     a,e
                ld      e,a
                jr      nc,ve_n
                inc     d
ve_n:           inc     hl
                djnz    ve_l
ve_d:           ex      de,hl
                pop     de
                ret
; A = 필요한 바이트 수 → CF = HL 뒤에 그만큼 남지 않음 (DE = 끝)
vb_avail:       ld      b,a
                ld      a,e
                sub     l
                ld      a,d
                sbc     a,h
                jr      nz,va_ok
                ld      a,e
                sub     l
                cp      b
                ret
va_ok:          or      a
                ret
; A = 이어지는 바이트 수 → A = 0 맞음 (HL 전진), 1 잘못됨, 2 덩어리 끝에서 잘림 (파일 끝이면 1)
vb_ucont:       ld      (vb_nc),a
                call    vb_avail
                jr      nc,vu_chk
                ld      a,(v_inc)
                or      a
                ld      a,2
                ret     z
                ld      a,1
                ret
vu_chk:         ld      a,(vb_nc)
                ld      b,a
                push    hl
vu_l:           ld      a,(hl)
                and     0C0h
                cp      80h
                jr      nz,vu_bad
                inc     hl
                djnz    vu_l
                pop     af
                xor     a
                ret
vu_bad:         pop     hl
                ld      a,1
                ret

; ---- 윗줄: 이름 + 위치/크기 (+ 인코딩)
v_head:         ld      hl,hbuf
                ld      de,hbuf+1
                ld      (hl),' '
                ld      bc,79
                ldir
                ld      hl,v_name
                ld      de,hbuf+1
                call    copy_z
                ld      a,(v_enc)
                or      a
                jr      z,vh_p
                ld      hl,txt_vu8
                dec     a
                jr      z,vh_e
                ld      hl,txt_vek
vh_e:           ld      de,hbuf+70
                call    copy_z
vh_p:           ld      de,hbuf+20
                ld      hl,(v_size+2)
                ld      a,h
                or      l
                jr      nz,vh_k
                ld      hl,(v_top)
                call    put_dec16
                ld      a,'/'
                ld      (de),a
                inc     de
                ld      hl,(v_size)
                call    put_dec16
                jr      vh_w
vh_k:           ld      hl,v_top
                call    v_kb
                call    put_dec16
                ld      a,'/'
                ld      (de),a
                inc     de
                push    de
                ld      hl,v_size
                call    v_kb
                pop     de
                call    put_dec16
                ld      a,'K'
                ld      (de),a
vh_w:           xor     a
                call    row_addr
                ld      de,hbuf
                ld      b,80
                jp      scr_write
; HL = 4 바이트 값의 주소 → HL = 킬로바이트 수
v_kb:           inc     hl
                ld      a,(hl)
                srl     a
                srl     a
                ld      c,a
                inc     hl
                ld      e,(hl)
                inc     hl
                ld      d,(hl)
                ex      de,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                ld      b,0
                add     hl,bc
                ret

; ---- 한 줄(화면 줄) 위로: top 앞 덩어리를 읽어 앞 줄의 시작에서 화면 줄 단위로 앞으로 따라가 마지막 줄 시작을 구함
v_up1:          ld      hl,(v_top)
                ld      de,(v_top+2)
                ld      a,h
                or      l
                or      d
                or      e
                ret     z
                ld      hl,(v_clen)
                ld      a,h
                or      l
                jr      z,vu_rd
                ld      hl,(v_top)              ; 이미 읽어 둔 곳을 쓸 수 있는가: d = top - coff
                ld      de,(v_coff)
                or      a
                sbc     hl,de
                ld      (v_d),hl
                ld      hl,(v_top+2)
                ld      de,(v_coff+2)
                sbc     hl,de
                jr      c,vu_rd
                ld      a,h
                or      l
                jr      nz,vu_rd
                ld      hl,(v_d)
                ld      de,(v_clen)
                or      a
                sbc     hl,de
                jr      c,vu_ok
                jr      z,vu_ok
                jr      vu_rd
vu_ok:          ld      hl,(v_d)
                ld      de,400
                or      a
                sbc     hl,de
                jr      nc,vu_use
                ld      hl,(v_coff)
                ld      de,(v_coff+2)
                ld      a,h
                or      l
                or      d
                or      e
                jr      nz,vu_rd
vu_use:         ld      hl,(v_d)
                ld      (v_n),hl
                jr      vu_scan
vu_rd:          ld      hl,(v_top)
                ld      de,(v_top+2)
                ld      bc,(v_chunk)
                or      a
                sbc     hl,bc
                jr      nc,vu_s1
                ld      a,d
                or      e
                jr      z,vu_z
                dec     de
vu_s1:          ld      (v_want),hl
                ld      (v_want+2),de
                jr      vu_ld
vu_z:           ld      hl,0
                ld      (v_want),hl
                ld      (v_want+2),hl
vu_ld:          call    v_load0
                ld      hl,(v_top)
                ld      de,(v_want)
                or      a
                sbc     hl,de
                ld      (v_n),hl
vu_scan:        ld      bc,(v_n)                ; BC = top 앞의 바이트 수 (VBUF 안)
                ld      a,b
                or      c
                ret     z
                ld      hl,VBUF
                add     hl,bc
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      nz,vs_c
                dec     bc                      ; 앞 줄 끝의 줄바꿈(CR LF) 을 뺌
                ld      a,b
                or      c
                jr      z,vs_s0
                dec     hl
                ld      a,(hl)
                cp      0Dh
                jr      nz,vs_c
                dec     bc
vs_c:           ld      hl,VBUF                 ; BC = 앞 줄 글의 끝 (번호)
                add     hl,bc
                ld      (v_end),hl
                ld      d,b
                ld      e,c
vs_f:           ld      a,d
                or      e
                jr      z,vs_l0
                dec     hl
                ld      a,(hl)
                cp      0Ah
                jr      z,vs_l
                dec     de
                jr      vs_f
vs_l:           inc     hl                      ; 줄바꿈 다음 = 앞 줄 처음
                jr      vs_w
vs_l0:          ld      hl,VBUF                 ; 덩어리 안에 줄바꿈이 없음: 덩어리 처음부터 (UTF-8 이면 이어지는 바이트는 건너뜀)
                ld      a,(v_enc)
                dec     a
                jr      nz,vs_w
vs_a:           ld      de,(v_end)
                push    hl
                or      a
                sbc     hl,de
                pop     hl
                jr      nc,vs_w
                ld      a,(hl)
                and     0C0h
                cp      80h
                jr      nz,vs_w
                inc     hl
                jr      vs_a
vs_s0:          ld      hl,VBUF
                ld      (v_end),hl
vs_w:           ld      a,1
                ld      (v_inc),a
                ld      (v_ptr),hl
vs_lp:          ld      hl,(v_ptr)
                ld      (v_s),hl
                ld      de,(v_end)
                or      a
                sbc     hl,de
                jr      nc,vs_dn
                call    v_row_build
                ld      hl,(v_ptr)
                ld      de,(v_end)
                or      a
                sbc     hl,de
                jr      c,vs_lp
vs_dn:          ld      hl,(v_s)
                ld      de,-VBUF
                add     hl,de
                ld      b,h
                ld      c,l
                ld      hl,(v_coff)
                add     hl,bc
                ld      (v_top),hl
                ld      hl,(v_coff+2)
                jr      nc,vs_t
                inc     hl
vs_t:           ld      (v_top+2),hl
                ret
; ---- 찾기 (0.12.52): Ctrl+F 로 글을 묻고(영문, 대소문자 무시, 바이트 비교 = UTF-8 한글은 못 찾음), n/F3 다음, p 이전.
;  파일을 2000 바이트씩 읽어 가며 훑음(덩어리 끝에 걸친 것은 겹쳐 읽음). 찾으면 그 줄(앞 줄바꿈 뒤)이 맨 위로 옴.
dv_find:        xor     a
                ld      (in_len),a
                ld      (inbuf),a
                ld      (il_first),a
                ld      a,2
                ld      (il_raw),a
                ld      hl,txt_vfq
                call    input_line
                push    af
                xor     a
                ld      (il_raw),a
                pop     af
                jp      nz,dv_rd
                ld      a,(in_len)
                or      a
                jp      z,dv_rd
                cp      33
                jr      c,fz_a
                ld      a,32
fz_a:           ld      (v_sl),a
                ld      b,a
                ld      hl,inbuf
                ld      de,v_sn
fz_c:           ld      a,(hl)
                call    up_a
                ld      (de),a
                inc     hl
                inc     de
                djnz    fz_c
                xor     a
                ld      (v_mv),a
                jr      dv_go0
dv_nxt:         xor     a
                jr      dv_s1
dv_prv:         ld      a,1
dv_s1:          ld      (v_dir),a
                ld      a,(v_sl)
                or      a
                jp      z,dv_find
                jr      dv_go
dv_go0:         xor     a
                ld      (v_dir),a
dv_go:          call    fz_run
                or      a
                jp      nz,dv_rd
                ld      hl,txt_vnf
                ld      de,txt_vok
                call    popup_ask
dv_rd:          call    clear_screen            ; 입력·알림 상자 자국을 지우고 다시 그림
                call    v_foot_draw
                jp      dv_draw
; → A = 1 찾음(v_top 이 그 줄 처음) / 0 없음
fz_run:         ld      hl,0
                ld      (v_clen),hl             ; VBUF 를 덮어쓰므로 보기 캐시 무효
                ld      a,(v_dir)
                or      a
                jp      nz,fz_back
                call    fz_cur
                jr      nz,fz_ft
                ld      hl,(v_mp)               ; 지난 번 찾은 자리 + 1 부터
                ld      de,(v_mp+2)
                inc     hl
                ld      a,h
                or      l
                jr      nz,fz_fs
                inc     de
                jr      fz_fs
fz_ft:          ld      hl,(v_top)
                ld      de,(v_top+2)
fz_fs:          ld      (v_sp),hl
                ld      (v_sp+2),de
fz_fl:          call    fz_rd
                ld      a,(v_sl)
                ld      e,a
                ld      d,0
                or      a
                sbc     hl,de
                jp      c,fz_no
                inc     hl
                ld      b,h
                ld      c,l
                ld      hl,VBUF
                call    fz_scan
                jp      nz,fz_hit
                ld      hl,(v_sn_n)
                ld      de,2000
                or      a
                sbc     hl,de
                jp      c,fz_no                 ; 덜 읽음 = 파일 끝
                ld      a,(v_sl)
                dec     a
                ld      e,a
                ld      d,0
                ld      hl,2000
                or      a
                sbc     hl,de
                ex      de,hl
                ld      hl,(v_sp)
                add     hl,de
                ld      (v_sp),hl
                jr      nc,fz_fl
                ld      hl,(v_sp+2)
                inc     hl
                ld      (v_sp+2),hl
                jr      fz_fl
; 뒤로: Q(= v_q) 앞의 가장 가까운 것을 찾음
fz_back:        call    fz_cur
                jr      nz,fz_bt
                ld      hl,(v_mp)
                ld      de,(v_mp+2)
                jr      fz_bs
fz_bt:          ld      hl,(v_top)
                ld      de,(v_top+2)
fz_bs:          ld      (v_q),hl
                ld      (v_q+2),de
                ld      a,h
                or      l
                or      d
                or      e
                jp      z,fz_no
fz_bl:          ld      a,(v_sl)                ; S = Q - (2000 - (길이-1)), 0 아래면 0
                dec     a
                ld      e,a
                ld      d,0
                ld      hl,2000
                or      a
                sbc     hl,de
                ex      de,hl
                ld      hl,(v_q)
                or      a
                sbc     hl,de
                ld      de,(v_q+2)
                jr      nc,fz_bn
                ld      a,d
                or      e
                jr      z,fz_bz
                dec     de
fz_bn:          ld      (v_sp),hl
                ld      (v_sp+2),de
                jr      fz_br
fz_bz:          ld      hl,0
                ld      (v_sp),hl
                ld      (v_sp+2),hl
fz_br:          call    fz_rd
                ld      de,(v_sp)               ; BC = Q - S (시작 후보 수)
                ld      hl,(v_q)
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l
                ld      hl,(v_sn_n)             ; 읽은 것 속에서 끝까지 들어가는 시작 수 = n - 길이 + 1
                ld      a,(v_sl)
                ld      e,a
                ld      d,0
                or      a
                sbc     hl,de
                jr      c,fz_b0
                inc     hl
                or      a
                sbc     hl,bc
                jr      nc,fz_bm                ; 충분: BC 그대로
                add     hl,bc
                ld      b,h
                ld      c,l
                jr      fz_bm
fz_b0:          ld      bc,0
fz_bm:          ld      hl,VBUF
                call    fz_scan
                jp      nz,fz_hit
                ld      hl,(v_sp)               ; S = 0 이면 더 없음
                ld      de,(v_sp+2)
                ld      a,h
                or      l
                or      d
                or      e
                jp      z,fz_no
                ld      hl,(v_sp)               ; Q = S 로 앞 덩어리
                ld      (v_q),hl
                ld      hl,(v_sp+2)
                ld      (v_q+2),hl
                jp      fz_bl
; 찾음: HL = VBUF 안 자리 → v_mp(절대 위치), v_top(그 줄 처음, 덩어리 앞이면 덩어리 처음)
fz_hit:         ld      de,VBUF
                or      a
                sbc     hl,de
                ld      b,h
                ld      c,l                     ; BC = 덩어리 안 번호
                ld      hl,(v_sp)
                add     hl,bc
                ld      (v_mp),hl
                ld      hl,(v_sp+2)
                jr      nc,fz_h1
                inc     hl
fz_h1:          ld      (v_mp+2),hl
                ld      hl,VBUF
                add     hl,bc
fz_hb:          ld      a,b
                or      c
                jr      z,fz_hs
                dec     hl
                dec     bc
                ld      a,(hl)
                cp      0Ah
                jr      nz,fz_hb
                inc     bc                      ; 줄바꿈 다음이 줄 처음
fz_hs:          ld      hl,(v_sp)
                add     hl,bc
                ld      (v_top),hl
                ld      hl,(v_sp+2)
                jr      nc,fz_h2
                inc     hl
fz_h2:          ld      (v_top+2),hl
                ld      hl,v_top
                ld      de,v_ft
                ld      bc,4
                ldir
                ld      a,1
                ld      (v_mv),a
                ret
fz_no:          xor     a
                ret
; v_sp 에서 2000 바이트를 VBUF 에 읽음 → HL = 읽은 수 (v_sn_n)
fz_rd:          ld      de,(v_sp+2)
                ld      hl,(v_sp)
                ld      a,(v_h)
                ld      b,a
                xor     a
                ld      c,F_SEEK
                call    BDOS
                ld      a,(v_h)
                ld      b,a
                ld      de,VBUF
                ld      hl,2000
                ld      c,F_READ
                call    BDOS
                cp      E_EOF
                jr      z,fz_rk
                or      a
                jr      z,fz_rk
                ld      hl,0
fz_rk:          ld      (v_sn_n),hl
                ret
; HL = 시작 번지, BC = 시작 후보 수 → NZ 이면 HL = 찾은 번지 (앞으로 = 첫 것, 뒤로 = 마지막 것)
fz_scan:        ld      de,0
                ld      (v_lh),de
fz_sl:          ld      a,b
                or      c
                jr      z,fz_se
                ld      a,(hl)
                call    up_a
                ld      d,a
                ld      a,(v_sn)
                cp      d
                jr      nz,fz_sn
                push    hl
                push    bc
                call    fz_cmp
                pop     bc
                pop     hl
                jr      nz,fz_sn
                ld      (v_lh),hl
                ld      a,(v_dir)
                or      a
                jr      z,fz_se
fz_sn:          inc     hl
                dec     bc
                jr      fz_sl
fz_se:          ld      hl,(v_lh)
                ld      a,h
                or      l
                ret
; HL = 후보 → Z 이면 글 전체가 맞음
fz_cmp:         ld      de,v_sn
                ld      a,(v_sl)
                ld      b,a
fz_ml:          ld      a,(hl)
                call    up_a
                ld      c,a
                ld      a,(de)
                cp      c
                ret     nz
                inc     hl
                inc     de
                djnz    fz_ml
                ret
; 지난 번 찾은 줄이 그대로 맨 위에 있으면 Z (거기서부터 이어 찾음)
fz_cur:         ld      a,(v_mv)
                or      a
                jr      nz,fz_cc
                inc     a
                ret
fz_cc:          ld      hl,v_top
                ld      de,v_ft
                ld      b,4
fz_ce:          ld      a,(de)
                cp      (hl)
                ret     nz
                inc     hl
                inc     de
                djnz    fz_ce
                ret
txt_vfq:        db      "Find text",0
txt_vnf:        db      "Text not found",0
txt_vok:        db      "[Enter] OK",0
txt_vfoot:      db      "Up/Dn line  Lt/Rt/Spc page  Home top  B end  ^F find  n/p next/prev  Esc close",0
v_sl:           db      0
v_dir:          db      0
v_mv:           db      0
v_sn:           ds      32
v_mp:           ds      4
v_ft:           ds      4
v_sp:           ds      4
v_q:            ds      4
v_sn_n:         dw      0
v_lh:           dw      0
txt_vu8:        db      "UTF-8",0
txt_vek:        db      "EUC-KR",0
v_h:            db      0
v_top:          ds      4
v_want:         ds      4
v_coff:         ds      4
v_clen:         dw      0
v_chunk:        dw      2000
v_bback:        dw      1760
v_size:         ds      4
v_nxt:          ds      4
v_aft:          ds      4
v_ptr:          dw      0
v_end:          dw      0
v_s:            dw      0
v_row:          db      0
v_more:         db      0
v_body:         db      22
v_foot:         db      23
v_enc:          db      0                       ; 0 = ASCII, 1 = UTF-8, 2 = EUC-KR
v_inc:          db      0                       ; 1 = 끝에서 잘린 글자도 잘못된 바이트로 봄
v_eof:          db      0
v_name:         ds      16
v_d:            dw      0
v_n:            dw      0
v_dst:          dw      0
v_dlt:          dw      0
vb_ld:          db      0
vb_nc:          db      0
vb_cs:          dw      0
vb_sp:          dw      0
vb_n:           dw      0
